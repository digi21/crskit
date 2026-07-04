#include "pch.h"

#include <format>
#include <variant>
#include <vector>

#include "Wkt2.h"

#include "AngularUnit.h"
#include "AxisInfo.h"
#include "AxisOrientationEnum.h"
#include "CompoundCoordinateSystem.h"
#include "CoordinateSystem.h"
#include "DatumType.h"
#include "Ellipsoid.h"
#include "GeocentricCoordinateSystem.h"
#include "GeographicCoordinateSystem.h"
#include "HorizontalDatum.h"
#include "IHorizontalDatum.h"
#include "IInfo.h"
#include "ILocalDatum.h"
#include "IProjection.h"
#include "IVerticalDatum.h"
#include "LinearUnit.h"
#include "LocalCoordinateSystem.h"
#include "LocalDatum.h"
#include "PrimeMeridian.h"
#include "ProjectedCoordinateSystem.h"
#include "Projection.h"
#include "Unit.h"
#include "VerticalCoordinateSystem.h"
#include "VerticalDatum.h"
#include "../OpenGisException.h"
#include "../Parameter.h"
#include "../StringUtil.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::Wkt
{
	namespace
	{
		// ---- emission helpers -------------------------------------------------------------------

		// WKT2 axis directions are lower-case (north, east, ...), unlike WKT1's capitalised names.
		auto Direction(AxisOrientationEnum orientation) -> std::string
		{
			switch (orientation)
			{
			case AxisOrientationEnum::North: return "north";
			case AxisOrientationEnum::South: return "south";
			case AxisOrientationEnum::East: return "east";
			case AxisOrientationEnum::West: return "west";
			case AxisOrientationEnum::Up: return "up";
			case AxisOrientationEnum::Down: return "down";
			default: return "unspecified";
			}
		}

		// ",ID[\"EPSG\",4326]" when the object carries an authority, otherwise empty.
		auto OptionalId(std::string const& authority, int code) -> std::string
		{
			if (authority.empty() || 0 == code)
				return "";
			return std::format(",ID[\"{}\",{}]", authority, code);
		}

		// The only emission difference between WKT2:2015 and WKT2:2019 is the geographic CRS keyword: 2015
		// expresses a geographic CRS as a geodetic CRS (GEODCRS / BASEGEODCRS), whereas 2019 added the
		// dedicated GEOGCRS / BASEGEOGCRS keywords. Everything else we emit is identical between the two.
		auto GeographicKeyword(WktVersion version) -> const char*
		{
			return version == WktVersion::Wkt2_2015 ? "GEODCRS" : "GEOGCRS";
		}

		auto BaseGeographicKeyword(WktVersion version) -> const char*
		{
			return version == WktVersion::Wkt2_2015 ? "BASEGEODCRS" : "BASEGEOGCRS";
		}

		auto AngleUnitWkt2(AngularUnit const& unit) -> std::string
		{
			return std::format("ANGLEUNIT[\"{}\",{:.16g}{}]",
				unit.GetName(), unit.GetRadiansPerUnit(), OptionalId(unit.GetAuthority(), unit.GetAuthorityCode()));
		}

		auto LengthUnitWkt2(LinearUnit const& unit) -> std::string
		{
			return std::format("LENGTHUNIT[\"{}\",{:.16g}{}]",
				unit.GetName(), unit.GetMetersPerUnit(), OptionalId(unit.GetAuthority(), unit.GetAuthorityCode()));
		}

		auto AnyUnitWkt2(AnyUnit const& unit) -> std::string
		{
			if (std::holds_alternative<AngularUnit>(unit))
				return AngleUnitWkt2(std::get<AngularUnit>(unit));
			return LengthUnitWkt2(std::get<LinearUnit>(unit));
		}

		auto EllipsoidWkt2(Ellipsoid const& ellipsoid) -> std::string
		{
			return std::format("ELLIPSOID[\"{}\",{:.16g},{:.16g},{}{}]",
				ellipsoid.GetName(),
				ellipsoid.GetSemiMajorAxis(),
				ellipsoid.GetInverseFlattening(),
				LengthUnitWkt2(ellipsoid.GetLinearUnit()),
				OptionalId(ellipsoid.GetAuthority(), ellipsoid.GetAuthorityCode()));
		}

		auto DatumWkt2(std::shared_ptr<IHorizontalDatum> const& datum) -> std::string
		{
			auto const info = std::dynamic_pointer_cast<IInfo>(datum);
			// WKT2 geodetic datum: DATUM["name",ELLIPSOID[...]]. The prime meridian and any datum-to-WGS84
			// transformation live outside the datum (PRIMEM is a CRS member; TOWGS84 becomes a BoundCRS),
			// so they are deliberately not emitted here.
			return std::format("DATUM[\"{}\",{}{}]",
				info->GetName(),
				EllipsoidWkt2(datum->GetEllipsoid()),
				OptionalId(info->GetAuthority(), info->GetAuthorityCode()));
		}

		auto PrimeMeridianWkt2(PrimeMeridian const& primeMeridian) -> std::string
		{
			return std::format("PRIMEM[\"{}\",{:.16g},{}{}]",
				primeMeridian.GetName(),
				primeMeridian.GetLongitude(),
				AngleUnitWkt2(primeMeridian.GetAngularUnit()),
				OptionalId(primeMeridian.GetAuthority(), primeMeridian.GetAuthorityCode()));
		}

		// CS[type,dim] followed by its AXIS list. The unit is factored out after the axes when every axis
		// shares it (the common 2D case); otherwise it is attached per axis (e.g. a 3D geographic CRS whose
		// horizontal axes are angular and whose height axis is linear).
		auto CsAndAxesWkt2(std::string const& csType, std::vector<AxisInfo> const& axes, std::vector<AnyUnit> const& units) -> std::string
		{
			auto const uniform = std::ranges::all_of(units, [&](auto const& u) { return u == units.front(); });

			std::string text = std::format("CS[{},{}]", csType, axes.size());
			for (size_t i = 0; i < axes.size(); ++i)
			{
				if (uniform)
					text += std::format(",AXIS[\"{}\",{},ORDER[{}]]", axes[i].GetName(), Direction(axes[i].GetOrientation()), i + 1);
				else
					text += std::format(",AXIS[\"{}\",{},ORDER[{}],{}]", axes[i].GetName(), Direction(axes[i].GetOrientation()), i + 1, AnyUnitWkt2(units[i]));
			}
			if (uniform && !units.empty())
				text += "," + AnyUnitWkt2(units.front());
			return text;
		}

		auto AxesOf(CoordinateSystem const& cs) -> std::vector<AxisInfo>
		{
			std::vector<AxisInfo> axes;
			for (auto i = 0; i < cs.GetDimension(); ++i)
				axes.push_back(cs.GetAxis(i));
			return axes;
		}

		// ---- parsing helpers --------------------------------------------------------------------

		// Reads ID["authority",code] (or the legacy AUTHORITY["authority","code"]) from a node, if present.
		auto ParseId(TokenWkt const& node, std::string& authority, int& code) -> void
		{
			authority.clear();
			code = 0;
			if (node.HasChild("ID"))
			{
				auto const& id = node.GetChild("ID");
				authority = id.GetChild(0).GetString();
				code = id.GetChild(1).GetInt();
			}
			else if (node.HasChild("AUTHORITY"))
			{
				auto const& id = node.GetChild("AUTHORITY");
				authority = id.GetChild(0).GetString();
				code = id.GetChild(1).GetInt();
			}
		}

		// First direct child whose keyword matches any of 'names' (case-insensitive); nullptr if none.
		auto FindChild(TokenWkt const& node, std::initializer_list<const char*> names) -> const TokenWkt*
		{
			for (auto const* name : names)
				if (node.HasChild(name))
					return &node.GetChild(name);
			return nullptr;
		}

		auto ParseAngleUnit(TokenWkt const& container, AngularUnit const& fallback) -> AngularUnit
		{
			auto const* unit = FindChild(container, {"ANGLEUNIT", "UNIT"});
			if (!unit)
				return fallback;

			std::string authority;
			int code{};
			ParseId(*unit, authority, code);
			return AngularUnit{unit->GetChild(0).GetString(), authority, code, "", "", unit->GetChild(1).GetDouble()};
		}

		auto ParseLengthUnit(TokenWkt const& container, LinearUnit const& fallback) -> LinearUnit
		{
			auto const* unit = FindChild(container, {"LENGTHUNIT", "UNIT"});
			if (!unit)
				return fallback;

			std::string authority;
			int code{};
			ParseId(*unit, authority, code);
			return LinearUnit{unit->GetChild(0).GetString(), authority, code, "", "", unit->GetChild(1).GetDouble()};
		}

		auto ParseEllipsoid(TokenWkt const& ellipsoidNode) -> Ellipsoid
		{
			std::string authority;
			int code{};
			ParseId(ellipsoidNode, authority, code);

			auto const unit = ParseLengthUnit(ellipsoidNode, LinearUnit{"metre", 1.0});
			return Ellipsoid::CreateFromSemiMajorAxisInverseFlattening(
				ellipsoidNode.GetChild(0).GetString(),
				authority,
				code,
				ellipsoidNode.GetChild(1).GetDouble(),
				ellipsoidNode.GetChild(2).GetDouble(),
				unit);
		}

		auto FromGeographicWkt2(TokenWkt const& root) -> std::shared_ptr<GeographicCoordinateSystem>
		{
			auto const name = root.GetChild(0).GetString();

			auto const* datumNode = FindChild(root, {"DATUM", "GEODETICDATUM", "TRF"});
			if (!datumNode)
				throw WktParseException("DATUM");

			auto const* ellipsoidNode = FindChild(*datumNode, {"ELLIPSOID", "SPHEROID"});
			if (!ellipsoidNode)
				throw WktParseException("ELLIPSOID");
			auto const ellipsoid = ParseEllipsoid(*ellipsoidNode);

			std::string datumAuthority;
			int datumCode{};
			ParseId(*datumNode, datumAuthority, datumCode);
			auto const datum = std::make_shared<HorizontalDatum>(
				datumNode->GetChild(0).GetString(), datumAuthority, datumCode, "", "",
				DatumType::HD_Geocentric, ellipsoid, nullptr);

			AngularUnit const degree{"degree", "EPSG", 9122, "", "", 0.0174532925199433};
			auto const angularUnit = ParseAngleUnit(root, degree);

			PrimeMeridian primeMeridian{"Greenwich", "EPSG", 8901, "", "", angularUnit, 0.0};
			if (auto const* primemNode = FindChild(root, {"PRIMEM", "PRIMEMERIDIAN"}))
			{
				std::string pmAuthority;
				int pmCode{};
				ParseId(*primemNode, pmAuthority, pmCode);
				primeMeridian = PrimeMeridian{
					primemNode->GetChild(0).GetString(), pmAuthority, pmCode, "", "",
					ParseAngleUnit(*primemNode, angularUnit),
					primemNode->GetChild(1).GetDouble()};
			}

			auto axes = AxisInfo::FromWkt(root);
			if (axes.empty())
			{
				axes.resize(2);
				axes[0] = AxisInfo{"Lat", AxisOrientationEnum::North};
				axes[1] = AxisInfo{"Lon", AxisOrientationEnum::East};
			}

			std::vector<AnyUnit> units;
			units.resize(axes.size());
			units[0] = angularUnit;
			units[1] = angularUnit;
			if (axes.size() == 3)
				units[2] = ParseLengthUnit(root, LinearUnit{"metre", 1.0});

			std::string authority;
			int code{};
			ParseId(root, authority, code);

			return std::make_shared<GeographicCoordinateSystem>(
				name, authority, code, "", "", angularUnit, datum, primeMeridian, axes, units);
		}

		// A WKT2 geodetic CRS is geographic when its coordinate system is ellipsoidal; a Cartesian/spherical
		// CS makes it geocentric (handled in a later increment).
		auto IsEllipsoidalCs(TokenWkt const& root) -> bool
		{
			if (!root.HasChild("CS"))
				return true; // CS missing: assume geographic (the only kind implemented so far).
			return 0 == compareNoCase(root.GetChild("CS").GetChild(0).GetString().c_str(), "ellipsoidal");
		}

		// ---- projected CRS: OGC <-> EPSG method/parameter mapping ----------------------------------

		enum class ParamUnit { Angular, Length, Scale };

		struct Wkt2Param
		{
			const char* ogcName;   // OGC/WKT1 parameter name as stored in IProjection
			const char* epsgName;  // EPSG/WKT2 parameter name
			int epsgCode;          // EPSG parameter code
			ParamUnit unit;
		};

		struct Wkt2Method
		{
			const char* ogcClass;              // IProjection::GetClassName()
			const char* epsgName;              // EPSG/WKT2 method name
			int epsgCode;                      // EPSG operation method code
			std::vector<Wkt2Param> parameters; // in EPSG sort order
		};

		// Static map-projection method table, generated from the EPSG dataset (method/parameter names,
		// codes and order) cross-checked with the built-in registry (OGC class + parameter names). It is the
		// single source of truth for both WKT2 emission and parsing; a method not listed here is reported as
		// unsupported rather than emitted/parsed incorrectly. Note that the same OGC parameter name maps to
		// different EPSG codes depending on the method (e.g. latitude_of_origin is 8801 "natural origin" for
		// Transverse Mercator but 8821 "false origin" for Lambert Conic Conformal 2SP).
		auto Methods() -> std::vector<Wkt2Method> const&
		{
			using U = ParamUnit;
			static const std::vector<Wkt2Method> methods = {
				{"Transverse_Mercator", "Transverse Mercator", 9807, {
					{"latitude_of_origin", "Latitude of natural origin", 8801, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"scale_factor", "Scale factor at natural origin", 8805, U::Scale},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"Transverse_Mercator_South_Orientated", "Transverse Mercator (South Orientated)", 9808, {
					{"latitude_of_origin", "Latitude of natural origin", 8801, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"scale_factor", "Scale factor at natural origin", 8805, U::Scale},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"Lambert_Conformal_Conic_1SP", "Lambert Conic Conformal (1SP)", 9801, {
					{"latitude_of_origin", "Latitude of natural origin", 8801, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"scale_factor", "Scale factor at natural origin", 8805, U::Scale},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"Lambert_Conformal_Conic_2SP", "Lambert Conic Conformal (2SP)", 9802, {
					{"latitude_of_origin", "Latitude of false origin", 8821, U::Angular},
					{"central_meridian", "Longitude of false origin", 8822, U::Angular},
					{"standard_parallel1", "Latitude of 1st standard parallel", 8823, U::Angular},
					{"standard_parallel2", "Latitude of 2nd standard parallel", 8824, U::Angular},
					{"false_easting", "Easting at false origin", 8826, U::Length},
					{"false_northing", "Northing at false origin", 8827, U::Length}}},
				{"Mercator_1SP", "Mercator (variant A)", 9804, {
					{"latitude_of_origin", "Latitude of natural origin", 8801, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"scale_factor", "Scale factor at natural origin", 8805, U::Scale},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"Mercator_2SP", "Mercator (variant B)", 9805, {
					{"standard_parallel_1", "Latitude of 1st standard parallel", 8823, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"Oblique_Stereographic", "Oblique Stereographic", 9809, {
					{"latitude_of_origin", "Latitude of natural origin", 8801, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"scale_factor", "Scale factor at natural origin", 8805, U::Scale},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"Albers_Conic_Equal_Area", "Albers Equal Area", 9822, {
					{"latitude_of_center", "Latitude of false origin", 8821, U::Angular},
					{"longitude_of_center", "Longitude of false origin", 8822, U::Angular},
					{"standard_parallel1", "Latitude of 1st standard parallel", 8823, U::Angular},
					{"standard_parallel2", "Latitude of 2nd standard parallel", 8824, U::Angular},
					{"false_easting", "Easting at false origin", 8826, U::Length},
					{"false_northing", "Northing at false origin", 8827, U::Length}}},
				{"polyconic", "American Polyconic", 9818, {
					{"latitude_of_origin", "Latitude of natural origin", 8801, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"Cassini_Soldner", "Cassini-Soldner", 9806, {
					{"latitude_of_origin", "Latitude of natural origin", 8801, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"hotine_oblique_mercator", "Hotine Oblique Mercator (variant A)", 9812, {
					{"latitude_of_center", "Latitude of projection centre", 8811, U::Angular},
					{"longitude_of_center", "Longitude of projection centre", 8812, U::Angular},
					{"azimuth", "Azimuth at projection centre", 8813, U::Angular},
					{"rectified_grid_angle", "Angle from Rectified to Skew Grid", 8814, U::Angular},
					{"scale_factor", "Scale factor at projection centre", 8815, U::Scale},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"oblique_mercator", "Hotine Oblique Mercator (variant B)", 9815, {
					{"latitude_of_center", "Latitude of projection centre", 8811, U::Angular},
					{"longitude_of_center", "Longitude of projection centre", 8812, U::Angular},
					{"azimuth", "Azimuth at projection centre", 8813, U::Angular},
					{"rectified_grid_angle", "Angle from Rectified to Skew Grid", 8814, U::Angular},
					{"scale_factor", "Scale factor at projection centre", 8815, U::Scale},
					{"false_easting", "Easting at projection centre", 8816, U::Length},
					{"false_northing", "Northing at projection centre", 8817, U::Length}}},
				{"Lambert_Azimuthal_Equal_Area", "Lambert Azimuthal Equal Area", 9820, {
					{"latitude_of_center", "Latitude of natural origin", 8801, U::Angular},
					{"longitude_of_center", "Longitude of natural origin", 8802, U::Angular},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
				{"Popular_Visualisation_Pseudo_Mercator", "Popular Visualisation Pseudo Mercator", 1024, {
					{"latitude_of_origin", "Latitude of natural origin", 8801, U::Angular},
					{"central_meridian", "Longitude of natural origin", 8802, U::Angular},
					{"false_easting", "False easting", 8806, U::Length},
					{"false_northing", "False northing", 8807, U::Length}}},
			};
			return methods;
		}

		auto FindMethodByOgcClass(std::string const& className) -> Wkt2Method const*
		{
			for (auto const& m : Methods())
				if (0 == compareNoCase(m.ogcClass, className.c_str()))
					return &m;
			return nullptr;
		}

		auto FindMethodByEpsgCode(int code) -> Wkt2Method const*
		{
			for (auto const& m : Methods())
				if (m.epsgCode == code)
					return &m;
			return nullptr;
		}

		auto FindMethodByEpsgName(std::string const& name) -> Wkt2Method const*
		{
			for (auto const& m : Methods())
				if (0 == compareNoCase(m.epsgName, name.c_str()))
					return &m;
			return nullptr;
		}

		auto DegreeAngleUnitWkt2() -> std::string { return "ANGLEUNIT[\"degree\",0.0174532925199433]"; }
		auto ScaleUnitWkt2() -> std::string { return "SCALEUNIT[\"unity\",1]"; }

		// The base geographic CRS of a projected CRS: like GEOGCRS but with the BASEGEOGCRS keyword and no
		// coordinate system / axes (those belong to the projected CRS).
		auto BaseGeographicWkt2(GeographicCoordinateSystem const& gcs, WktVersion version) -> std::string
		{
			return std::format("{}[\"{}\",{},{}{}]",
				BaseGeographicKeyword(version),
				gcs.GetName(),
				DatumWkt2(gcs.GetHorizontalDatum()),
				PrimeMeridianWkt2(gcs.GetPrimeMeridian()),
				OptionalId(gcs.GetAuthority(), gcs.GetAuthorityCode()));
		}

		auto FromProjectedWkt2(TokenWkt const& root) -> std::shared_ptr<ProjectedCoordinateSystem>
		{
			auto const name = root.GetChild(0).GetString();

			auto const* baseNode = FindChild(root, {"BASEGEOGCRS", "BASEGEOGRAPHICCRS", "BASEGEODCRS", "BASEGEODETICCRS"});
			if (!baseNode)
				throw WktParseException("BASEGEOGCRS");
			auto const baseGcs = FromGeographicWkt2(*baseNode);

			auto const* conversionNode = FindChild(root, {"CONVERSION", "DERIVINGCONVERSION"});
			if (!conversionNode)
				throw WktParseException("CONVERSION");

			auto const* methodNode = FindChild(*conversionNode, {"METHOD", "PROJECTION"});
			if (!methodNode)
				throw WktParseException("METHOD");

			std::string methodAuthority;
			int methodCode{};
			ParseId(*methodNode, methodAuthority, methodCode);
			auto const* method = methodCode ? FindMethodByEpsgCode(methodCode) : FindMethodByEpsgName(methodNode->GetChild(0).GetString());
			if (!method)
				throw UnsupportedFormatException(std::format("WKT2 parsing: projection method '{}' is not supported yet.",
					static_cast<const char*>(methodNode->GetChild(0).GetString().c_str())));

			std::vector<Parameter> parameters;
			for (size_t i = 0; i < conversionNode->GetChildCount(); ++i)
			{
				auto const& child = conversionNode->GetChild(i);
				if (0 != compareNoCase(child.GetName().c_str(), "PARAMETER"))
					continue;

				std::string paramAuthority;
				int paramCode{};
				ParseId(child, paramAuthority, paramCode);
				auto const value = child.GetChild(1).GetDouble();

				Wkt2Param const* tableParam{};
				for (auto const& p : method->parameters)
					if ((paramCode && p.epsgCode == paramCode) || 0 == compareNoCase(p.epsgName, child.GetChild(0).GetString().c_str()))
					{
						tableParam = &p;
						break;
					}
				if (tableParam)
					parameters.emplace_back(std::string(tableParam->ogcName), value);
			}

			auto const projection = std::make_shared<Projection>(std::string(method->ogcClass), parameters);
			auto const linearUnit = ParseLengthUnit(root, LinearUnit{"metre", 1.0});

			auto axes = AxisInfo::FromWkt(root);
			if (axes.size() < 2)
			{
				axes.resize(2);
				axes[0] = AxisInfo{"E", AxisOrientationEnum::East};
				axes[1] = AxisInfo{"N", AxisOrientationEnum::North};
			}

			std::string authority;
			int code{};
			ParseId(root, authority, code);

			return std::make_shared<ProjectedCoordinateSystem>(
				name, authority, code, "", "", baseGcs, projection, linearUnit, axes[0], axes[1]);
		}

		auto FromGeocentricWkt2(TokenWkt const& root) -> std::shared_ptr<GeocentricCoordinateSystem>
		{
			auto const name = root.GetChild(0).GetString();

			auto const* datumNode = FindChild(root, {"DATUM", "GEODETICDATUM", "TRF"});
			if (!datumNode)
				throw WktParseException("DATUM");
			auto const* ellipsoidNode = FindChild(*datumNode, {"ELLIPSOID", "SPHEROID"});
			if (!ellipsoidNode)
				throw WktParseException("ELLIPSOID");

			std::string datumAuthority;
			int datumCode{};
			ParseId(*datumNode, datumAuthority, datumCode);
			auto const datum = std::make_shared<HorizontalDatum>(
				datumNode->GetChild(0).GetString(), datumAuthority, datumCode, "", "",
				DatumType::HD_Geocentric, ParseEllipsoid(*ellipsoidNode), nullptr);

			AngularUnit const degree{"degree", "EPSG", 9122, "", "", 0.0174532925199433};
			PrimeMeridian primeMeridian{"Greenwich", "EPSG", 8901, "", "", degree, 0.0};
			if (auto const* primemNode = FindChild(root, {"PRIMEM", "PRIMEMERIDIAN"}))
			{
				std::string pmAuthority;
				int pmCode{};
				ParseId(*primemNode, pmAuthority, pmCode);
				primeMeridian = PrimeMeridian{
					primemNode->GetChild(0).GetString(), pmAuthority, pmCode, "", "",
					ParseAngleUnit(*primemNode, degree), primemNode->GetChild(1).GetDouble()};
			}

			auto const linearUnit = ParseLengthUnit(root, LinearUnit{"metre", 1.0});

			auto axes = AxisInfo::FromWkt(root);
			if (axes.size() < 3)
			{
				axes.resize(3);
				axes[0] = AxisInfo{"X", AxisOrientationEnum::Other};
				axes[1] = AxisInfo{"Y", AxisOrientationEnum::Other};
				axes[2] = AxisInfo{"Z", AxisOrientationEnum::Other};
			}

			std::string authority;
			int code{};
			ParseId(root, authority, code);

			return std::make_shared<GeocentricCoordinateSystem>(
				name, authority, code, "", "", linearUnit, datum, primeMeridian, axes[0], axes[1], axes[2]);
		}

		auto FromVerticalWkt2(TokenWkt const& root) -> std::shared_ptr<VerticalCoordinateSystem>
		{
			auto const name = root.GetChild(0).GetString();

			auto const* datumNode = FindChild(root, {"VDATUM", "VERTICALDATUM", "VERT_DATUM"});
			if (!datumNode)
				throw WktParseException("VDATUM");
			std::string datumAuthority;
			int datumCode{};
			ParseId(*datumNode, datumAuthority, datumCode);
			auto const datum = std::make_shared<VerticalDatum>(
				datumNode->GetChild(0).GetString(), datumAuthority, datumCode, "", "",
				DatumType::VD_GeoidModelDerived);

			auto const linearUnit = ParseLengthUnit(root, LinearUnit{"metre", 1.0});

			auto axes = AxisInfo::FromWkt(root);
			if (axes.empty())
			{
				axes.resize(1);
				axes[0] = AxisInfo{"H", AxisOrientationEnum::Up};
			}

			std::string authority;
			int code{};
			ParseId(root, authority, code);

			return std::make_shared<VerticalCoordinateSystem>(
				name, authority, code, "", "", linearUnit, datum, axes[0]);
		}

		auto FromLocalWkt2(TokenWkt const& root) -> std::shared_ptr<LocalCoordinateSystem>
		{
			auto const name = root.GetChild(0).GetString();

			auto const* datumNode = FindChild(root, {"EDATUM", "ENGINEERINGDATUM", "LOCAL_DATUM"});
			if (!datumNode)
				throw WktParseException("EDATUM");
			std::string datumAuthority;
			int datumCode{};
			ParseId(*datumNode, datumAuthority, datumCode);
			auto const datum = std::make_shared<LocalDatum>(
				datumNode->GetChild(0).GetString(), DatumType::LD_Min, datumAuthority, datumCode);

			auto const linearUnit = ParseLengthUnit(root, LinearUnit{"metre", 1.0});

			auto axes = AxisInfo::FromWkt(root);
			if (axes.empty())
			{
				axes.resize(2);
				axes[0] = AxisInfo{"X", AxisOrientationEnum::East};
				axes[1] = AxisInfo{"Y", AxisOrientationEnum::North};
			}

			return std::make_shared<LocalCoordinateSystem>(name, datum, AnyUnit{linearUnit}, axes);
		}

		// Compound CRS: parses each component CRS (themselves WKT2) recursively through the public FromWkt2.
		auto FromCompoundWkt2(TokenWkt const& root) -> std::shared_ptr<CompoundCoordinateSystem>
		{
			auto const name = root.GetChild(0).GetString();
			auto const head = FromWkt2(root.GetChild(1));
			auto const tail = FromWkt2(root.GetChild(2));

			std::string authority;
			int code{};
			ParseId(root, authority, code);

			return std::make_shared<CompoundCoordinateSystem>(name, authority, code, "", "", head, tail);
		}
	}

	auto IsWkt2Keyword(std::string const& keyword) -> bool
	{
		static constexpr const char* kKeywords[] = {
			"GEOGCRS", "GEOGRAPHICCRS", "GEODCRS", "GEODETICCRS",
			"PROJCRS", "PROJECTEDCRS", "VERTCRS", "VERTICALCRS",
			"COMPOUNDCRS", "ENGCRS", "ENGINEERINGCRS",
		};
		return std::ranges::any_of(kKeywords, [&](auto const* k) { return 0 == compareNoCase(keyword.c_str(), k); });
	}

	auto FromWkt2(TokenWkt const& root) -> std::shared_ptr<CoordinateSystem>
	{
		auto const keyword = root.GetName();

		if (0 == compareNoCase(keyword.c_str(), "GEOGCRS") || 0 == compareNoCase(keyword.c_str(), "GEOGRAPHICCRS"))
			return FromGeographicWkt2(root);

		if (0 == compareNoCase(keyword.c_str(), "GEODCRS") || 0 == compareNoCase(keyword.c_str(), "GEODETICCRS"))
			return IsEllipsoidalCs(root) ? std::static_pointer_cast<CoordinateSystem>(FromGeographicWkt2(root))
				: std::static_pointer_cast<CoordinateSystem>(FromGeocentricWkt2(root));

		if (0 == compareNoCase(keyword.c_str(), "PROJCRS") || 0 == compareNoCase(keyword.c_str(), "PROJECTEDCRS"))
			return FromProjectedWkt2(root);

		if (0 == compareNoCase(keyword.c_str(), "VERTCRS") || 0 == compareNoCase(keyword.c_str(), "VERTICALCRS"))
			return FromVerticalWkt2(root);

		if (0 == compareNoCase(keyword.c_str(), "COMPOUNDCRS"))
			return FromCompoundWkt2(root);

		if (0 == compareNoCase(keyword.c_str(), "ENGCRS") || 0 == compareNoCase(keyword.c_str(), "ENGINEERINGCRS"))
			return FromLocalWkt2(root);

		throw UnsupportedFormatException(std::format("WKT2 keyword '{}' is not supported yet.", static_cast<const char*>(keyword.c_str())));
	}

	auto ToWkt2(GeographicCoordinateSystem const& gcs, WktVersion version) -> std::string
	{
		return std::format("{}[\"{}\",{},{},{}{}]",
			GeographicKeyword(version),
			gcs.GetName(),
			DatumWkt2(gcs.GetHorizontalDatum()),
			PrimeMeridianWkt2(gcs.GetPrimeMeridian()),
			CsAndAxesWkt2("ellipsoidal", AxesOf(gcs), gcs.GetUnits()),
			OptionalId(gcs.GetAuthority(), gcs.GetAuthorityCode()));
	}

	auto ToWkt2(ProjectedCoordinateSystem const& pcs, WktVersion version) -> std::string
	{
		auto const projection = pcs.GetProjection();
		auto const* method = FindMethodByOgcClass(projection->GetClassName());
		if (!method)
			throw UnsupportedFormatException(std::format("WKT2 emission: projection method '{}' is not supported yet.",
				static_cast<const char*>(projection->GetClassName().c_str())));

		auto const baseGcs = std::dynamic_pointer_cast<GeographicCoordinateSystem>(pcs.GetGeographicCoordinateSystem());
		auto const linearUnit = pcs.GetLinearUnit();

		std::string parameters;
		for (auto const& p : method->parameters)
		{
			auto const value = std::get<double>(projection->GetParameter(p.ogcName).GetValue());
			auto const unit = p.unit == ParamUnit::Angular ? DegreeAngleUnitWkt2()
				: p.unit == ParamUnit::Length ? LengthUnitWkt2(linearUnit)
				: ScaleUnitWkt2();
			parameters += std::format(",PARAMETER[\"{}\",{:.16g},{},ID[\"EPSG\",{}]]", p.epsgName, value, unit, p.epsgCode);
		}

		auto const conversion = std::format("CONVERSION[\"unnamed\",METHOD[\"{}\",ID[\"EPSG\",{}]]{}]",
			method->epsgName, method->epsgCode, parameters);

		std::vector<AnyUnit> const csUnits(static_cast<size_t>(pcs.GetDimension()), AnyUnit{linearUnit});
		return std::format("PROJCRS[\"{}\",{},{},{}{}]",
			pcs.GetName(),
			BaseGeographicWkt2(*baseGcs, version),
			conversion,
			CsAndAxesWkt2("Cartesian", AxesOf(pcs), csUnits),
			OptionalId(pcs.GetAuthority(), pcs.GetAuthorityCode()));
	}

	auto ToWkt2(GeocentricCoordinateSystem const& gcs, [[maybe_unused]] WktVersion version) -> std::string
	{
		// A geocentric CRS is GEODCRS in both WKT2:2015 and 2019, so the version does not change the output.
		// Geocentric axes use the dedicated geocentricX/Y/Z directions (our model stores them with the
		// generic 'Other' orientation, so they cannot come from the orientation enum).
		static const char* const kDirections[] = {"geocentricX", "geocentricY", "geocentricZ"};
		auto const axes = AxesOf(gcs);

		std::string cs = "CS[Cartesian,3]";
		for (size_t i = 0; i < axes.size() && i < 3; ++i)
			cs += std::format(",AXIS[\"{}\",{},ORDER[{}]]", axes[i].GetName(), kDirections[i], i + 1);
		cs += "," + LengthUnitWkt2(gcs.GetLinearUnit());

		return std::format("GEODCRS[\"{}\",{},{},{}{}]",
			gcs.GetName(),
			DatumWkt2(gcs.GetHorizontalDatum()),
			PrimeMeridianWkt2(gcs.GetPrimeMeridian()),
			cs,
			OptionalId(gcs.GetAuthority(), gcs.GetAuthorityCode()));
	}

	auto ToWkt2(VerticalCoordinateSystem const& vcs, [[maybe_unused]] WktVersion version) -> std::string
	{
		auto const info = std::dynamic_pointer_cast<IInfo>(vcs.GetVerticalDatum());
		auto const datum = std::format("VDATUM[\"{}\"{}]", info->GetName(), OptionalId(info->GetAuthority(), info->GetAuthorityCode()));

		auto const axes = AxesOf(vcs);
		std::vector<AnyUnit> const units(axes.size(), AnyUnit{vcs.GetVerticalUnit()});

		return std::format("VERTCRS[\"{}\",{},{}{}]",
			vcs.GetName(),
			datum,
			CsAndAxesWkt2("vertical", axes, units),
			OptionalId(vcs.GetAuthority(), vcs.GetAuthorityCode()));
	}

	auto ToWkt2(CompoundCoordinateSystem const& ccs, WktVersion version) -> std::string
	{
		// Each component CRS is emitted in the same WKT2 version explicitly (no reliance on global state).
		auto const head = std::dynamic_pointer_cast<CoordinateSystem>(ccs.GetHeadCS());
		auto const tail = std::dynamic_pointer_cast<CoordinateSystem>(ccs.GetTailCS());

		return std::format("COMPOUNDCRS[\"{}\",{},{}{}]",
			ccs.GetName(),
			head->GetWkt(version),
			tail->GetWkt(version),
			OptionalId(ccs.GetAuthority(), ccs.GetAuthorityCode()));
	}

	auto ToWkt2(LocalCoordinateSystem const& lcs, [[maybe_unused]] WktVersion version) -> std::string
	{
		auto const info = std::dynamic_pointer_cast<IInfo>(lcs.GetLocalDatum());
		auto const datum = std::format("EDATUM[\"{}\"{}]", info->GetName(), OptionalId(info->GetAuthority(), info->GetAuthorityCode()));

		auto const axes = AxesOf(lcs);
		std::vector<AnyUnit> const units(axes.size(), lcs.GetUnits(0));

		return std::format("ENGCRS[\"{}\",{},{}{}]",
			lcs.GetName(),
			datum,
			CsAndAxesWkt2("Cartesian", axes, units),
			OptionalId(lcs.GetAuthority(), lcs.GetAuthorityCode()));
	}
}
