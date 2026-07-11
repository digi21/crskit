// Python binding for CrsKit (pybind11).
//
// A thin, Pythonic layer over the C++ API: snake_case names, read-only properties, NumPy for
// bulk transformation, structural equality as __eq__ and the library's typed exceptions mapped
// to a Python exception hierarchy. The C++ library remains the reference API; nothing here
// changes its design.

#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "CrsKit.h"
#include "SqliteProvider.h"

namespace py = pybind11;

using CrsKit::CoordinateSystems::CoordinateSystem;
using CrsKit::CoordinateTransformations::ICoordinateTransformation;
using CrsKit::CoordinateTransformations::IMathTransform;

namespace
{
	// The exception type carrying the grid-file details. Set once at module init and deliberately
	// leaked (a static py::object would be destroyed after the interpreter is gone).
	PyObject* gGridFileNotFoundError{};

	auto IsInitialized() -> bool
	{
		return nullptr != CrsKit::GetCurrentEnvironment()->Provider;
	}

	auto RequireInitialized() -> void
	{
		if (!IsInitialized())
			throw CrsKit::OpenGisException{ "crskit is not initialised: call crskit.init(epsg_database) first." };
	}

	auto CrsRepr(CoordinateSystem const& crs) -> std::string
	{
		auto const code = crs.GetAuthorityCode();
		auto const authority = code == CrsKit::CoordinateSystems::UnknownAuthorityCode
			? std::string{ "no code" }
			: std::format("{}:{}", crs.GetAuthority(), code);

		return std::format("<Crs {} \"{}\" ({}D)>", authority, crs.GetName(), crs.GetDimension());
	}

	// Transforms a NumPy array of points: either a single point of shape (source_dimension,) or a
	// buffer of N points of shape (N, source_dimension). Returns the same shape with the target
	// dimension. The GIL is released while the C++ transform runs, so other threads keep working.
	auto TransformPoints(ICoordinateTransformation const& transformation,
		py::array_t<double, py::array::c_style | py::array::forcecast> const& points) -> py::array_t<double>
	{
		auto const mathTransform = transformation.GetMathTransform();
		auto const sourceDimension = static_cast<py::ssize_t>(mathTransform->GetSourceDimension());
		auto const targetDimension = static_cast<py::ssize_t>(mathTransform->GetTargetDimension());

		auto const info = points.request();
		if (info.ndim != 1 && info.ndim != 2)
			throw CrsKit::DimensionMismatchException{ std::format("expected an array of shape (source_dimension,) or (n, source_dimension); got {} dimensions.", info.ndim) };

		auto const columns = info.ndim == 1 ? info.shape[0] : info.shape[1];
		if (columns != sourceDimension)
			throw CrsKit::DimensionMismatchException{ std::format("the transformation takes {}D points, but the array has {} columns.", sourceDimension, columns) };

		auto const count = info.ndim == 1 ? py::ssize_t{ 1 } : info.shape[0];

		std::vector<double> transformed;
		{
			py::gil_scoped_release const release;
			transformed = mathTransform->TransformPoints(
				std::span<double const>{ static_cast<double const*>(info.ptr), static_cast<size_t>(count * sourceDimension) });
		}

		auto result = info.ndim == 1
			? py::array_t<double>(std::vector<py::ssize_t>{ targetDimension })
			: py::array_t<double>(std::vector<py::ssize_t>{ count, targetDimension });

		std::copy(transformed.begin(), transformed.end(), result.mutable_data());
		return result;
	}
}

PYBIND11_MODULE(_crskit, m)
{
	m.doc() = "CrsKit: coordinate reference systems and coordinate transformations (EPSG, OGC WKT 1 and WKT 2).";

	// ---------------------------------------------------------------------------------------
	// Exceptions. The C++ hierarchy maps one-to-one: every typed exception derives from
	// CrsError, so `except crskit.CrsError` still catches everything the library throws.
	// Registration order matters: pybind11 tries the translators in reverse order, so the base
	// goes first and the most specific type last.
	// ---------------------------------------------------------------------------------------
	auto const crsError = py::register_exception<CrsKit::OpenGisException>(m, "CrsError", PyExc_RuntimeError);
	py::register_exception<CrsKit::AuthorityCodeNotFoundException>(m, "AuthorityCodeNotFoundError", crsError.ptr());
	py::register_exception<CrsKit::WktParseException>(m, "WktParseError", crsError.ptr());
	py::register_exception<CrsKit::TransformationNotFoundException>(m, "TransformationNotFoundError", crsError.ptr());
	py::register_exception<CrsKit::DimensionMismatchException>(m, "DimensionMismatchError", crsError.ptr());
	py::register_exception<CrsKit::CoordinateOutsideDomainException>(m, "CoordinateOutsideDomainError", crsError.ptr());
	py::register_exception<CrsKit::UnsupportedFormatException>(m, "UnsupportedFormatError", crsError.ptr());

	// GridFileNotFoundError needs a translator of its own: the point of the exception is the data it
	// carries (which grid file, looked up where, for which EPSG operation), so the caller can go and
	// fetch the file. Registered last => tried first.
	gGridFileNotFoundError = py::exception<CrsKit::GridFileNotFoundException>(m, "GridFileNotFoundError", crsError.ptr()).release().ptr();

	py::register_exception_translator([](std::exception_ptr p)
	{
		try
		{
			if (p)
				std::rethrow_exception(p);
		}
		catch (CrsKit::GridFileNotFoundException const& e)
		{
			auto const type = py::reinterpret_borrow<py::object>(gGridFileNotFoundError);
			auto instance = type(e.what());
			instance.attr("grid_file") = e.gridFileName;
			instance.attr("searched_path") = e.searchedPath;
			instance.attr("operation_code") = e.operationCode ? py::cast(e.operationCode) : py::none();
			instance.attr("operation_name") = e.operationName;
			instance.attr("area_of_use") = e.areaOfUse;
			instance.attr("information_source") = e.informationSource;

			PyErr_SetObject(type.ptr(), instance.ptr());
		}
	});

	// ---------------------------------------------------------------------------------------
	// WKT flavours
	// ---------------------------------------------------------------------------------------
	py::enum_<CrsKit::WktVersion>(m, "WktVersion", "The Well-Known Text flavour to_wkt() emits.")
		.value("WKT1", CrsKit::WktVersion::Wkt1, "OGC 01-009 (Simple Features).")
		.value("WKT1_ESRI", CrsKit::WktVersion::Wkt1Esri, "ESRI WKT 1, without the EPSG authority code.")
		.value("WKT1_ESRI_WITH_EPSG_CODE", CrsKit::WktVersion::Wkt1EsriWithEpsgCode, "ESRI WKT 1, with the EPSG authority code.")
		.value("WKT2_2015", CrsKit::WktVersion::Wkt2_2015, "ISO 19162:2015 / OGC 12-063r5.")
		.value("WKT2_2019", CrsKit::WktVersion::Wkt2_2019, "ISO 19162:2019 / OGC 18-010r7.");

	// ---------------------------------------------------------------------------------------
	// Crs
	// ---------------------------------------------------------------------------------------
	py::class_<CoordinateSystem, std::shared_ptr<CoordinateSystem>> crs(m, "Crs",
		"A coordinate reference system. Build one with crs_from_epsg() or crs_from_wkt().");

	crs
		.def_property_readonly("name", &CoordinateSystem::GetName)
		.def_property_readonly("authority", &CoordinateSystem::GetAuthority)
		.def_property_readonly("authority_code",
			[](CoordinateSystem const& self) -> py::object
			{
				auto const code = self.GetAuthorityCode();
				return code == CrsKit::CoordinateSystems::UnknownAuthorityCode ? py::none() : py::cast(code);
			},
			"The authority code (the EPSG code), or None when the system carries none.")
		.def_property_readonly("dimension", &CoordinateSystem::GetDimension)
		.def_property_readonly("remarks", &CoordinateSystem::GetRemarks)
		.def_property_readonly("axes",
			[](CoordinateSystem const& self)
			{
				py::list axes;
				for (auto dimension = 0; dimension < self.GetDimension(); ++dimension)
				{
					auto const axis = self.GetAxis(dimension);
					axes.append(py::make_tuple(axis.GetName(), CrsKit::CoordinateSystems::ToString(axis.GetOrientation())));
				}
				return axes;
			},
			"The axes, as (name, orientation) pairs in axis order.")
		.def("to_wkt", [](CoordinateSystem const& self, CrsKit::WktVersion version) { return self.GetWkt(version); },
			py::arg("version") = CrsKit::WktVersion::Wkt1,
			"The system's Well-Known Text in the requested flavour.")
		.def("compare",
			[](std::shared_ptr<CoordinateSystem> const& self, std::shared_ptr<CoordinateSystem> const& other)
			{
				return CrsKit::CoordinateSystems::CompareCrs(self, other);
			},
			py::arg("other"),
			"Graded structural similarity with another system, 0..100 (100 == equivalent).")
		.def("__eq__",
			[](std::shared_ptr<CoordinateSystem> const& self, py::object const& other) -> py::object
			{
				if (!py::isinstance<CoordinateSystem>(other))
					return py::reinterpret_borrow<py::object>(py::handle(Py_NotImplemented));

				return py::cast(CrsKit::CoordinateSystems::AreEquivalent(self, other.cast<std::shared_ptr<CoordinateSystem>>()));
			},
			py::arg("other"), py::is_operator(),
			"Structural equivalence: same mathematical definition (datum, prime meridian, projection and\n"
			"parameters, units), ignoring name, authority and axis order.")
		.def("__repr__", &CrsRepr);

	// Structural equality has no cheap, consistent hash (two equivalent systems must hash alike), so
	// a Crs is deliberately unhashable rather than silently hashing by identity.
	crs.attr("__hash__") = py::none();

	// ---------------------------------------------------------------------------------------
	// Transformation
	// ---------------------------------------------------------------------------------------
	py::class_<ICoordinateTransformation, std::shared_ptr<ICoordinateTransformation>>(m, "Transformation",
		"A coordinate transformation between two systems. Build one with transformation().")
		.def_property_readonly("name", &ICoordinateTransformation::GetName)
		.def_property_readonly("authority", &ICoordinateTransformation::GetAuthority)
		.def_property_readonly("authority_code",
			[](ICoordinateTransformation const& self) -> py::object
			{
				auto const code = self.GetAuthorityCode();
				return code.empty() ? py::none() : py::cast(code);
			},
			"The EPSG code of the coordinate operation, or None when it is not one.")
		.def_property_readonly("area_of_use", &ICoordinateTransformation::GetAreaOfUse)
		.def_property_readonly("remarks", &ICoordinateTransformation::GetRemarks)
		.def_property_readonly("source",
			[](ICoordinateTransformation const& self)
			{
				return std::dynamic_pointer_cast<CoordinateSystem>(self.GetSourceCS());
			})
		.def_property_readonly("target",
			[](ICoordinateTransformation const& self)
			{
				return std::dynamic_pointer_cast<CoordinateSystem>(self.GetTargetCS());
			})
		.def_property_readonly("source_dimension",
			[](ICoordinateTransformation const& self) { return self.GetMathTransform()->GetSourceDimension(); })
		.def_property_readonly("target_dimension",
			[](ICoordinateTransformation const& self) { return self.GetMathTransform()->GetTargetDimension(); })
		.def_property_readonly("is_identity",
			[](ICoordinateTransformation const& self) { return self.GetMathTransform()->GetIsIdentity(); })
		.def("transform",
			[](ICoordinateTransformation const& self, std::vector<double> const& point)
			{
				auto const mathTransform = self.GetMathTransform();
				if (static_cast<int>(point.size()) != mathTransform->GetSourceDimension())
					throw CrsKit::DimensionMismatchException{ std::format("the transformation takes {}D points, but got {} coordinates.", mathTransform->GetSourceDimension(), point.size()) };

				py::gil_scoped_release const release;
				return mathTransform->Transform(point);
			},
			py::arg("point"),
			"Transforms a single point, given as a sequence of coordinates.")
		.def("transform_points", &TransformPoints, py::arg("points"),
			"Transforms a NumPy array of points, of shape (source_dimension,) or (n, source_dimension).")
		.def("__repr__",
			[](ICoordinateTransformation const& self)
			{
				return std::format("<Transformation \"{}\">", self.GetName());
			});

	// ---------------------------------------------------------------------------------------
	// Module-level API
	// ---------------------------------------------------------------------------------------
	m.def("init",
		[](std::filesystem::path const& epsg_database, std::optional<std::filesystem::path> const& data_directory)
		{
			if (!std::filesystem::exists(epsg_database))
				throw CrsKit::OpenGisException{ std::format("EPSG database not found: '{}'.", epsg_database.string()) };

			if (!CrsKit::Initialize(std::make_shared<CrsKit::Epsg::SQliteProvider>(epsg_database.string())))
				throw CrsKit::OpenGisException{ std::format("could not initialise crskit with the EPSG database '{}'.", epsg_database.string()) };

			// Where the grid files (geoids, NTv2, NADCON) are looked up. Defaults to the folder holding
			// the EPSG database, which is where they usually live.
			auto const directory = data_directory.value_or(epsg_database.parent_path());
			CrsKit::GetDefaultContext()->dataDirectory = (directory / "").string();
		},
		py::arg("epsg_database"), py::arg("data_directory") = py::none(),
		"Initialises the library against an EPSG SQLite database. Must be called before anything else,\n"
		"unless a host application embedding the interpreter has already initialised CrsKit itself (see\n"
		"is_initialized()), in which case calling this replaces its EPSG database with the given one.\n"
		"data_directory is where grid files (geoids, NTv2, NADCON) are looked up; it defaults to the\n"
		"folder holding the database.");

	m.def("is_initialized", &IsInitialized,
		"Whether the library already has an EPSG database. True without calling init() when a host\n"
		"application initialised the very same CrsKit library this module links against.");

	m.def("epsg_version",
		[]
		{
			RequireInitialized();
			return CrsKit::GetEpsgVersion();
		},
		"The EPSG Geodetic Parameter Dataset version of the active database (e.g. '12.057').");

	m.def("crs_from_epsg",
		[](int code)
		{
			RequireInitialized();
			py::gil_scoped_release const release;
			return CrsKit::GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code);
		},
		py::arg("code"),
		"The coordinate reference system with this EPSG code.");

	m.def("crs_from_wkt",
		[](std::string const& wkt)
		{
			RequireInitialized();
			py::gil_scoped_release const release;
			return CrsKit::GetCoordinateSystemFactory()->CreateFromWkt(wkt);
		},
		py::arg("wkt"),
		"The coordinate reference system described by this Well-Known Text (WKT 1 or WKT 2).");

	m.def("transformation",
		[](std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target)
		{
			RequireInitialized();

			std::shared_ptr<ICoordinateTransformation> transformation;
			{
				py::gil_scoped_release const release;
				transformation = CrsKit::GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target);
			}

			if (!transformation)
				throw CrsKit::TransformationNotFoundException{ std::format("no coordinate operation found from '{}' to '{}'.", source->GetName(), target->GetName()) };

			return transformation;
		},
		py::arg("source"), py::arg("target"),
		"The transformation from one coordinate reference system to another.");
}
