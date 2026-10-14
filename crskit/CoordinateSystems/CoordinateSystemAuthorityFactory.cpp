#include "pch.h"
#include "BaseInfo.h"
#include "Ellipsoid.h"
#include "Projection.h"
#include "CoordinateSystemAuthorityFactory.h"
#include "LinearUnit.h"
#include "DatumType.h"
#include "HorizontalDatum.h"
#include "VerticalDatum.h"
#include "PrimeMeridian.h"
#include "AngularUnit.h"
#include "CoordinateSystem.h"
#include "HorizontalCoordinateSystem.h"
#include "GeographicCoordinateSystem.h"
#include "ProjectedCoordinateSystem.h"
#include "VerticalCoordinateSystem.h"
#include "GeocentricCoordinateSystem.h"
#include "../ExtensionManager.h"
#include "../EsriProjectionEngine.h"
#include "../CrsContext.h"

using namespace CrsKit::Positioning;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;
using namespace std;

namespace CrsKit::Epsg
{
	namespace
	{
		// The EPSG code a CRS code resolves to through CrsContext::aliases (itself when it has no alias).
		auto ResolveCrsAlias(int code) -> int
		{
			return GetDefaultContext()->aliases.ResolveCrs(code);
		}
	}

	CoordinateSystemAuthorityFactory::CoordinateSystemAuthorityFactory(std::shared_ptr<Epsg::IAuthorityProvider> const& authorityProvider)
		: _provider{ authorityProvider }
	{
	}

	auto CoordinateSystemAuthorityFactory::GetAuthority() const -> std::string
	{
		return "EPSG";
	}

	auto CoordinateSystemAuthorityFactory::CreateCoordinateSystem(int code) const->std::shared_ptr<CoordinateSystems::CoordinateSystem>
	{
		code = ResolveCrsAlias(code);
		auto const coordinateSystemType = _provider->GetCoordinateSystemType(code);
		if (0 == compareNoCase(coordinateSystemType.c_str(), "projected"))
			return CreateProjectedCoordinateSystem(code);
		if (0 == compareNoCase(coordinateSystemType.c_str(), "geographic 2D"))
			return CreateGeographicCoordinateSystem(code);
		if (0 == compareNoCase(coordinateSystemType.c_str(), "geographic 3D"))
			return CreateGeographicCoordinateSystem(code);
		if (0 == compareNoCase(coordinateSystemType.c_str(), "vertical"))
			return CreateVerticalCoordinateSystem(code);
		if (0 == compareNoCase(coordinateSystemType.c_str(), "geocentric"))
			return CreateGeocentricCoordinateSystem(code);

		// An empty kind means the catalogue holds no CRS with that code at all; any other kind is one
		// this factory cannot build (compound, engineering...). Two different failures, two exceptions.
		if (coordinateSystemType.empty())
			throw AuthorityCodeNotFoundException(std::format("Could not locate the coordinate system with code: {}", code));

		throw UnsupportedFormatException(std::format("The coordinate system {} is of kind '{}', which cannot be created from its code alone.", code, coordinateSystemType));
	}

	auto CoordinateSystemAuthorityFactory::CreateProjectedCoordinateSystem(int code) const -> std::shared_ptr<ProjectedCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
		// ProjectedCoordinateSystem holds two axes: a projected 3D CRS (EPSG 9895, with ellipsoidal height) is rejected.
		auto const axis = CreateAxisInformation(code);
		if (axis.size() > 2)
			throw UnsupportedFormatException(std::format("The projected coordinate system {} has {} axes; only projected systems with 2 axes are supported.", code, axis.size()));

		return CreateProjectedCoordinateSystem(code, axis);
	}

	auto CoordinateSystemAuthorityFactory::CreateProjectedCoordinateSystem(int code, std::vector<AxisInfo> const& axis) const -> std::shared_ptr<ProjectedCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
		auto const coordinateSystemName = _provider->GetCoordinateReferenceSystemName(code, IsEsri(GetDefaultContext()->wktVersion));
		if (coordinateSystemName.empty())
			throw AuthorityCodeNotFoundException(std::format("Could not locate the coordinate system with code: {}", code) );

		auto const coordinateSystemType = _provider->GetCoordinateSystemType(code);
		if (0 != compareNoCase(coordinateSystemType.c_str(), "projected"))
			throw runtime_error( std::format("The {} coordinate system is not projected.", code) );

		if (axis.size() != 2)
			throw UnsupportedFormatException(std::format("A projected coordinate system takes 2 axes, not {}.", axis.size()));

		auto const units = CreateUnitsInformation(code);
		if (units.empty())
			throw OpenGisException(std::format("The coordinate system of the projected CRS {} has no axis unit.", code));

		auto geographicCoordinateSystem = CreateGeographicCoordinateSystem(_provider->GetGeographicCrsCodeForProjectedCrs(code));
		auto const operationCode = _provider->GetOperationCodeForCrs(code);
		auto const algorithmCode = _provider->GetAlgorithmCodeForOperation(operationCode);
		auto const projection = CreateProjection(operationCode, algorithmCode);

		return make_shared<ProjectedCoordinateSystem>(
			coordinateSystemName,
			"EPSG",
			code,
			_provider->GetAlias(code),
			_provider->GetCoordinateReferenceSystemInfo(code),
			geographicCoordinateSystem,
			projection,
			std::get<LinearUnit>(units[0]),
			axis[0],
			axis[1]);
	}

	auto CoordinateSystemAuthorityFactory::CreateProjectedCoordinateSystem(int projectionCode, int geographicCrsCode) const -> std::shared_ptr<ProjectedCoordinateSystem>
	{
		geographicCrsCode = ResolveCrsAlias(geographicCrsCode);
		auto const maximum = _provider->GetProjectedCrsCountFromProjConvCodeAndBaseCrsCode(projectionCode, geographicCrsCode);
		if (0 == maximum)
			throw runtime_error("No projected coordinate system with the specified parameters has been located." );
		if (1 != maximum)
			throw runtime_error("Multiple reference coordinate systems have been located with the specified parameters" );

		auto const code = _provider->GetCrsCodeFromProjConvCodeAndBaseCrsCode(projectionCode, geographicCrsCode);
		return CreateProjectedCoordinateSystem(code);
	}

	auto CoordinateSystemAuthorityFactory::CreateAxisInformation(int code) const -> std::vector<AxisInfo>
	{
		auto const coordinateSystemCode = _provider->GetCoordinateSystemCodeForCrs(code);
		return GetAxisOfCoordinateSystem(coordinateSystemCode);
	}

	auto CoordinateSystemAuthorityFactory::CreateUnitsInformation(int code) const -> std::vector<AnyUnit>
	{
		auto const coordinateSystemCode = _provider->GetCoordinateSystemCodeForCrs(code);
		auto const axisUnitCodes = _provider->GetCoordinateSystemAxisUnitCodes(coordinateSystemCode);

		std::vector<AnyUnit> units;
		units.resize(axisUnitCodes.size());

		std::ranges::transform(axisUnitCodes, units.begin(),
			[this](auto code) { return CreateUnit(code); });

		return units;
	}

	auto CoordinateSystemAuthorityFactory::CreateUnit(int code) const -> AnyUnit
	{
		auto const name = _provider->GetUnitName(code);
		if (name.empty())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate unit with authority code: {}", code) );

		try
		{
			if (_provider->GetUnitIsLength(code))
				return CreateLinearUnit(code);
			return CreateAngularUnit(code);
		}
		catch (std::exception const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the unit with authority code {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreateGeographicCoordinateSystem(int code) const -> std::shared_ptr<GeographicCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
		return CreateGeographicCoordinateSystem(code, CreateAxisInformation(code));
	}

	auto CoordinateSystemAuthorityFactory::CreateGeographicCoordinateSystem(int code, std::vector<AxisInfo> const& axis) const -> std::shared_ptr<GeographicCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
		auto const coordinateSystemName = _provider->GetCoordinateReferenceSystemName(code, IsEsri(GetDefaultContext()->wktVersion));
		if (coordinateSystemName.empty())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate coordinate system with authority code: {}", code) );

		if (axis.size() != 2 && axis.size() != 3 && axis.size() != 0)
			throw runtime_error("The AXIS parameter must have either zero or two axes." );

		try
		{
			auto const coordinateSystemType = _provider->GetCoordinateSystemType(code);
			if (0 != compareNoCase(coordinateSystemType.c_str(), "geographic 2D") && 0 != compareNoCase(coordinateSystemType.c_str(), "geographic 3D"))
				throw runtime_error(std::format("The coordinate system {} is not geographic.", code) );

			auto units = CreateUnitsInformation(code);

			auto const datumCode = _provider->GetDatumCodeForCrs(code);

			auto horizontalDatum = CreateHorizontalDatum(datumCode);
			auto meridian = CreatePrimeMeridian(_provider->GetMeridianCodeForDatum(datumCode));

			return make_shared<GeographicCoordinateSystem>(
				coordinateSystemName,
				"EPSG",
				code,
				_provider->GetAlias(code),
				_provider->GetCoordinateReferenceSystemInfo(code),
				std::get<AngularUnit>(units[0]),
				horizontalDatum,
				meridian,
				axis,
				units);
		}
		catch (OpenGisException const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the coordinate system {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreateCompoundCoordinateSystem(int horizontal, int vertical) const->std::shared_ptr<CoordinateSystems::CompoundCoordinateSystem>
	{
		auto const h = CreateHorizontalCoordinateSystem(horizontal);
		auto const v = CreateVerticalCoordinateSystem(vertical);

		return make_shared<CompoundCoordinateSystem>(
			std::format("{} + {}", h->GetName().c_str(), v->GetName().c_str()),
			h,
			v);
	}

	auto CoordinateSystemAuthorityFactory::CreateHorizontalDatum(int code) const -> std::shared_ptr<IHorizontalDatum>
	{
		auto const datumName = _provider->GetDatumName(code);
		if (datumName.empty())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate datum with authority code: {}", code));

		try
		{
			return make_shared<HorizontalDatum>(
				datumName,
				"EPSG",
				code,
				_provider->GetAlias(code),
				_provider->GetInfoDatum(code),
				DatumType::HD_Geocentric,
				CreateEllipsoid(_provider->GetEllipsoidCodeForDatum(code)),
				ResolveWgs84ConversionInfo(code));
		}
		catch (std::exception const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the datum with authority code {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::ResolveWgs84ConversionInfo(int datumCode) const -> std::shared_ptr<CoordinateSystems::Wgs84ConversionInfo>
	{
		// WGS 84 geographic 2D CRS: the target of every TOWGS84 datum shift.
		constexpr int kWgs84GeographicCrs = 4326;

		// Methods expressible as a 7-parameter Helmert (and therefore as TOWGS84):
		// 9603 geocentric translations (3 params), 9606 Position Vector, 9607 Coordinate Frame.
		constexpr int kGeocentricTranslations = 9603;
		constexpr int kPositionVector = 9606;
		constexpr int kCoordinateFrame = 9607;

		// nullptr when the datum has no Helmert transformation to WGS 84. A database failure throws.
		auto const geographicCrs = _provider->GetGeographicCrsCodeForDatum(datumCode);
		if (0 == geographicCrs || kWgs84GeographicCrs == geographicCrs)
			return nullptr;  // No geographic CRS for the datum, or the datum is WGS 84 itself.

		// Candidates are already ordered by accuracy (best first) and exclude deprecated operations.
		for (auto const& operation : _provider->OperationCodesToTransformSystemAtoB(geographicCrs, kWgs84GeographicCrs))
		{
			if (kGeocentricTranslations != operation.MethodCode && kPositionVector != operation.MethodCode && kCoordinateFrame != operation.MethodCode)
				continue;  // Grid- or Molodensky-based transformations cannot be written as TOWGS84.

			auto const parameters = _provider->GetTransformationParameters(operation.Code, operation.MethodCode);
			auto const value = [&](size_t i)
			{
				if (i >= parameters.size())
					return 0.0;
				if (!std::holds_alternative<double>(parameters[i]))
					throw OpenGisException(std::format("Parameter {} of the transformation {} is not a number.", i, operation.Code));
				return std::get<double>(parameters[i]);
			};

			if (kGeocentricTranslations == operation.MethodCode)
				return std::make_shared<Wgs84ConversionInfo>(value(0), value(1), value(2), 0.0, 0.0, 0.0, 0.0);

			// The provider returns rotations in decimal degrees, but TOWGS84 (Wgs84ConversionInfo)
			// expresses them in arc-seconds, so we scale them up. 9606 is already Position Vector;
			// 9607 (Coordinate Frame) is the same shift with the opposite rotation sign, so we also
			// negate the rotations to emit Position Vector, the convention WKT 1 TOWGS84 follows in
			// GDAL/PROJ. Translations (metres) and scale (ppm) need no conversion.
			constexpr double kDegreesToArcSeconds = 3600.0;
			auto const rotationFactor = ((kCoordinateFrame == operation.MethodCode) ? -1.0 : 1.0) * kDegreesToArcSeconds;
			return std::make_shared<Wgs84ConversionInfo>(
				value(0), value(1), value(2),
				rotationFactor * value(3), rotationFactor * value(4), rotationFactor * value(5),
				value(6));
		}

		return nullptr;
	}

	auto CoordinateSystemAuthorityFactory::CreateEllipsoid(int code) const -> Ellipsoid
	{
		auto const ellipsoidName = _provider->GetEllipsoidName(code);
		if (0 == ellipsoidName.size())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate ellipsoid with authority code: {}", code) );

		try
		{
			// The INV_FLATTENING field may be empty in the EPSG DB (e.g. Clarke 1880 (IGN),
			// code 7011, defined by semi-minor axis). ExecuteScalar returns 0.0 for a NULL
			// and an inverse flattening of 0 produces a -infinity semi-minor axis and NaN eccentricity.
			// Therefore, if there is no valid inverse flattening, we build it from the semi-minor axis.
			auto const inverseFlattening = _provider->GetInverseFlattening(code);
			if (inverseFlattening > 0.0)
				return Ellipsoid::CreateFromSemiMajorAxisInverseFlattening(
					_provider->GetEllipsoidName(code),
					"EPSG",
					code,
					_provider->GetSemiMajorAxis(code),
					inverseFlattening,
					CreateLinearUnit(_provider->GetEllipsoidLinearUnit(code)));

			return Ellipsoid::CreateFromSemiMajorAxisSemiMinorAxis(
				_provider->GetEllipsoidName(code),
				"EPSG",
				code,
				_provider->GetSemiMajorAxis(code),
				_provider->GetSemiMinorAxis(code),
				CreateLinearUnit(_provider->GetEllipsoidLinearUnit(code)));
		}
		catch (OpenGisException const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the ellipsoid with authority code {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreatePrimeMeridian(int code) const -> PrimeMeridian
	{
		auto const meridianName = _provider->GetMeridianName(code);
		if (0 == meridianName.size())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate meridian with authority code: {}", code));

		try
		{
			return PrimeMeridian{
				meridianName,
				"EPSG",
				code,
				_provider->GetAlias(code),
				_provider->GetMeridianInfo(code),
				CreateAngularUnit(_provider->GetMeridianAngularUnit(code)),
				_provider->GetMeridianLongitude(code)};
		}
		catch (std::exception const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the meridian with authority code {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreateLinearUnit(int code) const -> LinearUnit
	{
		auto const linearUnitName = _provider->GetUnitName(code);
		if (0 == linearUnitName.size())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate linear unit with authority code: {}", code));

		try
		{
			return LinearUnit{
				linearUnitName,
				"EPSG",
				code,
				_provider->GetAlias(code),
				_provider->GetUnitInfo(code),
				_provider->GetFactorToStandardUnit(code)};
		}
		catch (std::exception const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the linear unit with authority code {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreateAngularUnit(int code) const -> AngularUnit
	{
		auto const angularUnitName = _provider->GetUnitName(code);
		if (0 == angularUnitName.size())
			throw AuthorityCodeNotFoundException(std::format("Could not locate angular unit with authority code: {}", code) );

		try
		{
			return AngularUnit{
				angularUnitName,
				"EPSG",
				code,
				_provider->GetAlias(code),
				_provider->GetUnitInfo(code),
				_provider->GetFactorToStandardUnit(code)};
		}
		catch (std::exception const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the angular unit with authority code {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreateVerticalDatum(int code) const -> std::shared_ptr<IVerticalDatum>
	{
		auto const datumName = _provider->GetDatumName(code);
		if (0 == datumName.size())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate datum with authority code: {}", code));

		try
		{
			return make_shared<VerticalDatum>(
				datumName,
				"EPSG",
				code,
				_provider->GetAlias(code),
				_provider->GetInfoDatum(code),
				DatumType::VD_GeoidModelDerived);
		}
		catch (std::exception const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the datum with authority code {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreateVerticalCoordinateSystem(int code) const -> std::shared_ptr<VerticalCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
		auto const coordinateSystemName = _provider->GetCoordinateReferenceSystemName(code, IsEsri(GetDefaultContext()->wktVersion));
		if (coordinateSystemName.empty())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate coordinate system with authority code: {}", code));

		try
		{
			auto const coordinateSystemType = _provider->GetCoordinateSystemType(code);
			if (0 != compareNoCase(coordinateSystemType.c_str(), "vertical"))
				throw runtime_error(std::format("The coordinate system {} is not vertical.", code));

			auto const axis = CreateAxisInformation(code);
			auto const units = CreateUnitsInformation(code);
			// A derived vertical CRS has no DATUM_CODE and inherits the datum of its BASE_CRS_CODE, which can be
			// derived too: 6358 (NAVD88 depth (ftUS)) -> 6360 (NAVD88 height (ftUS)) -> 5703 (NAVD88 height).
			auto datumCode = _provider->GetDatumCodeForCrs(code);
			for (auto baseCrsCode = code, depth = 0; 0 == datumCode && depth < 8; ++depth)
			{
				baseCrsCode = _provider->GetGeographicCrsCodeForProjectedCrs(baseCrsCode);
				if (0 == baseCrsCode)
					break;
				datumCode = _provider->GetDatumCodeForCrs(baseCrsCode);
			}
			auto const verticalDatum = CreateVerticalDatum(datumCode);

			return make_shared<VerticalCoordinateSystem>(
				coordinateSystemName,
				"EPSG",
				code,
				_provider->GetAlias(code),
				_provider->GetCoordinateReferenceSystemInfo(code),
				std::get<LinearUnit>(units[0]),
				dynamic_pointer_cast<VerticalDatum>(verticalDatum),
				axis[0]);
		}
		catch (OpenGisException const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the coordinate system {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreateGeocentricCoordinateSystem(int code) const -> std::shared_ptr<GeocentricCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
		auto const coordinateSystemName = _provider->GetCoordinateReferenceSystemName(code, IsEsri(GetDefaultContext()->wktVersion));
		if (coordinateSystemName.empty())
			throw AuthorityCodeNotFoundException(std::format("Unable to locate coordinate system with authority code: {}", code));

		try
		{
			auto const coordinateSystemType = _provider->GetCoordinateSystemType(code);
			if (0 != compareNoCase(coordinateSystemType.c_str(), "geocentric"))
				throw runtime_error(std::format("The coordinate system {} is not geocentric.", code));

			auto const axis = CreateAxisInformation(code);
			auto const units = CreateUnitsInformation(code);
			auto const datumCode = _provider->GetDatumCodeForCrs(code);
			auto const horizontalDatum = CreateHorizontalDatum(datumCode);
			auto const meridian = CreatePrimeMeridian(_provider->GetMeridianCodeForDatum(datumCode));

			return make_shared<GeocentricCoordinateSystem>(
				coordinateSystemName,
				"EPSG",
				code,
				_provider->GetAlias(code),
				_provider->GetCoordinateReferenceSystemInfo(code),
				std::get<LinearUnit>(units[0]),
				dynamic_pointer_cast<HorizontalDatum>(horizontalDatum),
				meridian,
				axis[0],
				axis[1],
				axis[2]);
		}
		catch (OpenGisException const& e)
		{
			throw OpenGisException(std::format("An error was found when creating the coordinate system {}: {}", code, e.what()));
		}
	}

	auto CoordinateSystemAuthorityFactory::CreateHorizontalCoordinateSystem(int code) const -> std::shared_ptr<HorizontalCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
		// We check whether it is projected or geographic
		auto const coordinateSystemType = _provider->GetCoordinateSystemType(code);
		if (0 == compareNoCase(coordinateSystemType.c_str(), "projected"))
			return CreateProjectedCoordinateSystem(code);
		if (0 == compareNoCase(coordinateSystemType.c_str(), "geographic 2D"))
			return CreateGeographicCoordinateSystem(code);
		if (0 == compareNoCase(coordinateSystemType.c_str(), "geographic 3D"))
			return CreateGeographicCoordinateSystem(code);

		if (coordinateSystemType.empty())
			throw AuthorityCodeNotFoundException(std::format("Could not locate the coordinate system with code: {}", code));

		throw UnsupportedFormatException(std::format("The coordinate system {} is of kind '{}', which is not a horizontal system.", code, coordinateSystemType));
	}

	auto CoordinateSystemAuthorityFactory::EnumerateCoordinateSystems(std::string const& kind) const -> std::unordered_map<int, std::string>
	{
		return _provider->GetCoordinateReferenceSystemNames(kind);
	}

	auto CoordinateSystemAuthorityFactory::GetKindOfCoordinateSystem(int code) const->std::string
	{
		code = ResolveCrsAlias(code);
		return _provider->GetCoordinateSystemType(code);
	}

	auto CoordinateSystemAuthorityFactory::GetDescriptionAreaApplicationCrs(int code) const -> std::string
	{
		code = ResolveCrsAlias(code);
		return _provider->GetApplicationAreaDescription(code);
	}

	auto CoordinateSystemAuthorityFactory::GetCodeOfCoordinateSystemAssociatedWithCrs(int code) const -> int
	{
		code = ResolveCrsAlias(code);
		return _provider->GetCoordinateSystemCodeForCrs(code);
	}

	auto CoordinateSystemAuthorityFactory::GetNameOfCoordinateSystem(int code) const -> std::string
	{
		return _provider->GetCoordinateSystemName(code);
	}

	auto CoordinateSystemAuthorityFactory::GetAxisOfCoordinateSystem(int code) const->std::vector <CoordinateSystems::AxisInfo>
	{
		auto const names = _provider->GetCoordinateSystemAxisNames(code);
		auto const orientaciones = _provider->GetCoordinateSystemAxisOrientation(code);

		assert(names.size() == orientaciones.size());

		vector<AxisInfo> axis;
		axis.resize(names.size());

		for (auto i = 0u; i < names.size(); i++)
			axis[i] = AxisInfo(names[i], orientaciones[i]);
		return axis;
	}

	auto CoordinateSystemAuthorityFactory::GetCodeOfDatumAssociatedWithCrs(int code) const -> int
	{
		code = ResolveCrsAlias(code);
		return _provider->GetDatumCodeForCrs(code);
	}

	auto CoordinateSystemAuthorityFactory::GetNameOfDatum(int code) const -> std::string
	{
		return _provider->GetDatumName(code);
	}

	auto CoordinateSystemAuthorityFactory::GetOriginDescriptionOfDatum(int code) const -> std::string
	{
		return _provider->GetDatumOrigin(code);
	}

	auto CoordinateSystemAuthorityFactory::GetDescriptionAreaApplicationDatum(int code) const -> std::string
	{
		return _provider->GetDatumAreaOfUse(code);
	}

	auto CoordinateSystemAuthorityFactory::GetCodeOfPrimeMeridianAssociatedWithCrs(int code) const -> int
	{
		return _provider->GetMeridianCodeForDatum(code);
	}

	auto CoordinateSystemAuthorityFactory::GetNameOfPrimeMeridian(int code) const -> std::string
	{
		return _provider->GetMeridianName(code);
	}

	auto CoordinateSystemAuthorityFactory::GetCodeOfEllipsoidAssociatedWithDatum(int code) const -> int
	{
		return _provider->GetEllipsoidCodeForDatum(code);
	}

	auto CoordinateSystemAuthorityFactory::GetNameOfEllipsoid(int code) const->std::string
	{
		return _provider->GetEllipsoidName(code);
	}

	auto CoordinateSystemAuthorityFactory::GetNameOfAlgorithm(int code) const->std::string
	{
		return _provider->GetAlgorithmName(code);
	}


	auto CoordinateSystemAuthorityFactory::CreateProjection(int operationCode, int algorithmCode) const -> std::shared_ptr<IProjection>
	{
		auto const clase = [&]
		{
			try
			{
				return ExtensionManager::FindMathematicalOperation("EPSG", algorithmCode);
			}
			catch (UnsupportedFormatException const&)
			{
				throw UnsupportedFormatException(std::format("The projection method EPSG:{} '{}' of the conversion {} is not implemented.", algorithmCode, _provider->GetAlgorithmName(algorithmCode), operationCode));
			}
		}();
		auto const classParameters = ExtensionManager::GetMathOperationParameters(clase);
		auto const parameterValues = _provider->GetOperationParameters(operationCode, algorithmCode);

		// Each value goes to the class parameter with its EPSG code. A provider without codes (code 0) is mapped by
		// position, which relies on the class declaring its parameters in EPSG sort order.
		vector<CrsKit::Parameter> parameters;
		for (auto i = 0u; i < parameterValues.size(); i++)
		{
			auto const& [code, value] = parameterValues[i];
			auto const it = 0 == code
				? (i < classParameters.size() ? classParameters.begin() + i : classParameters.end())
				: std::ranges::find_if(classParameters, [code](auto const& p) { return p->EpsgCode == code; });
			if (it == classParameters.end())
				throw OpenGisException(std::format("The operation {} has a parameter (EPSG:{}) that the method EPSG:{} does not take.", operationCode, code, algorithmCode));

			if (ParameterType::String != (*it)->Type && !std::holds_alternative<double>(value))
				throw OpenGisException(std::format("The parameter '{}' of the operation {} has no numeric value.", (*it)->Name, operationCode));

			parameters.emplace_back((*it)->Name, value);
		}

		return make_shared<Projection>(
			ExtensionManager::GetClassificationName(clase),
			parameters);
	}

	auto CoordinateSystemAuthorityFactory::CreateGeographic2DSystemForGeographic3DSystem(int code, AxisInfo const& axisX, AxisInfo const& axisY) const -> std::shared_ptr<GeographicCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
#ifdef _DEBUG
		auto const coordinateSystemType = _provider->GetCoordinateSystemType(code);
		assert(0 == compareNoCase(coordinateSystemType.c_str(), "geographic 3D"));
#endif
		auto gcs = CreateGeographicCoordinateSystem(_provider->GetGeographic2DCrsCodeForGeographic3DCrs(code));
		gcs->SetAxis(0, axisX);
		gcs->SetAxis(1, axisY);
		return gcs;
	}

	auto CoordinateSystemAuthorityFactory::FindGeographicSystem3D(int code, AxisInfo const& axisX, AxisInfo const& axisY) const -> std::shared_ptr<GeographicCoordinateSystem>
	{
		code = ResolveCrsAlias(code);
#ifdef _DEBUG
		auto const coordinateSystemType = _provider->GetCoordinateSystemType(code);
		assert(0 == compareNoCase(coordinateSystemType.c_str(), "geographic 2D"));
#endif
		auto const crsCode3D = _provider->GetGeographic3DCrsCodeForGeographic2DCrs(code);
		if (0 == crsCode3D)
			return nullptr;

		auto gcs = CreateGeographicCoordinateSystem(crsCode3D);
		gcs->SetAxis(0, axisX);
		gcs->SetAxis(1, axisY);
		return gcs;
	}
}
