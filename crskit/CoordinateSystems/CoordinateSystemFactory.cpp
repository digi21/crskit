#include "pch.h"
#include "BaseInfo.h"
#include "Ellipsoid.h"
#include "CoordinateSystemFactory.h"
#include "PrimeMeridian.h"
#include "HorizontalDatum.h"
#include "VerticalDatum.h"
#include "CoordinateSystem.h"
#include "HorizontalCoordinateSystem.h"
#include "GeographicCoordinateSystem.h"
#include "ProjectedCoordinateSystem.h"
#include "VerticalCoordinateSystem.h"
#include "GeocentricCoordinateSystem.h"
#include "LocalDatum.h"
#include "LocalCoordinateSystem.h"
#include "CompoundCoordinateSystem.h"
#include "CoordinateSystemAuthorityFactory.h"
#include "Wkt2.h"
#include "../EsriProjectionEngine.h"

using namespace CrsKit::Epsg;
using namespace std;

namespace CrsKit::CoordinateSystems
{
	auto CatalogWins() -> bool
	{
		return GetDefaultContext()->parsePolicy == WktParsePolicy::CatalogWins;
	}

	auto CoordinateSystemFactory::CreateFrom(Wkt::TokenWkt const& token) -> std::shared_ptr<CoordinateSystem>
	{
		// WKT2 (ISO 19162) strings are recognised by their root keyword (GEOGCRS/PROJCRS/...) and parsed
		// by the dedicated WKT2 reader; WKT1 keeps the dispatch below.
		if (Wkt::IsWkt2Keyword(token.GetName()))
			return Wkt::FromWkt2(token);

		if (0 == compareNoCase(token.GetName().c_str(), "PROJCS"))
			return ProjectedCoordinateSystem::FromWkt(token);

		if (0 == compareNoCase(token.GetName().c_str(), "GEOGCS"))
			return GeographicCoordinateSystem::FromWkt(token);

		if (0 == compareNoCase(token.GetName().c_str(), "VERT_CS"))
			return VerticalCoordinateSystem::FromWkt(token);

		if (0 == compareNoCase(token.GetName().c_str(), "GEOCCS"))
			return GeocentricCoordinateSystem::FromWkt(token);

		if (0 == compareNoCase(token.GetName().c_str(), "LOCAL_CS"))
			return LocalCoordinateSystem::FromWkt(token);

		if (0 == compareNoCase(token.GetName().c_str(), "COMPD_CS"))
			return CompoundCoordinateSystem::FromWkt(token);

		throw WktParseException("node no reconocido");
	}

	auto CoordinateSystemFactory::CreateFromWkt(std::string const& wellKnownText) -> std::shared_ptr<CoordinateSystem>
	{
		std::string wkt(wellKnownText);
		Wkt::WktDeserializer const deserializadorWkt(wkt);
		auto const& root = deserializadorWkt.GetRootNode();

		// ESRI-flavoured WKT that carries an EPSG code resolves from the catalogue by that code (a clean
		// EPSG object rather than the ArcGIS names), regardless of the parse policy and never by matching
		// ESRI names. The ESRI compound (a VERTCS embedded in the PROJCS) is handled by the block below.
		if (EsriProjectionEngine::LooksLikeEsri(root) && !EsriProjectionEngine::IsEsriCompound(root))
			if (auto const code = Wkt::EpsgCode(root); 0 != code)
				return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code);

		// ESRI stores a compound CRS as a VERTCS embedded in the PROJCS. When both parts carry an EPSG code
		// the whole thing is rebuilt from the catalogue by those codes. (The ESRI VERTCS uses a non-OGC
		// structure -- VDATUM, Vertical_Shift -- that we cannot build from its pieces, so a compound without
		// codes falls through to the horizontal-only parse below.)
		if (EsriProjectionEngine::IsEsriCompound(root))
			if (auto const horizontalCode = Wkt::EpsgCode(root), verticalCode = Wkt::EpsgCode(root.GetChild("VERTCS")); 0 != horizontalCode && 0 != verticalCode)
				return GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(horizontalCode, verticalCode);

		if (CatalogWins() && root.GetName() == "PROJCS")
		{
			// The system may be Esri, compound or projected.
			//
			// If it is compound, in Esri format we do not have the COMPOUND_CS concept, so the vertical system
			// it will appear as a property of the PROJCS system, so here we convert it into a compound one.
			//
			// If it is not compound we are going to recreate it from the EPSG code, because the user may have 
			// the option to create OpenGIS-compatible systems is set, and therefore the name it will be assigned
			// this reference system will be the ArcInfo name and not the ArcGis name, which is the one the user expects to see.
			std::string verticalWkt;
			for (auto i = 0u; i < deserializadorWkt.GetRootNode().GetChildCount(); i++)
			{
				if (deserializadorWkt.GetRootNode().GetChild(i).GetName() == "VERTCS")
				{
					// It is an Esri system and also has a vertical component
					auto const comienzo = wkt.find(",VERTCS");
					verticalWkt = wkt.substr(wkt.size() - comienzo - 1);
					wkt = wkt.substr(0,comienzo);
					break;
				}
			}

			auto const horizontalEpsgCode = EsriProjectionEngine::FindHorizontalEpsgCode(wkt);
			auto const verticalEpsgCode = EsriProjectionEngine::FindVerticalEpsgCode(verticalWkt);
			if (0 != horizontalEpsgCode)
			{
				if (0 != verticalEpsgCode)
					return GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(horizontalEpsgCode, verticalEpsgCode);
				return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(horizontalEpsgCode);
			}
		}

		return CreateFrom(deserializadorWkt.GetRootNode());
	}

	auto CoordinateSystemFactory::CreateCompoundCoordinateSystem(std::string const& name, std::shared_ptr<CoordinateSystem> const& head, std::shared_ptr<CoordinateSystem> const& tail) -> std::shared_ptr<CompoundCoordinateSystem>
	{
		return make_shared<CompoundCoordinateSystem>(name, head, tail);
	}

	auto CoordinateSystemFactory::CreateEllipsoid(std::string const& name, double semiMajorAxis, double semiMinorAxis, LinearUnit const& linearUnit) -> Ellipsoid
	{
		return Ellipsoid::CreateFromSemiMajorAxisSemiMinorAxis(name, "", 0, semiMajorAxis, semiMinorAxis, linearUnit);
	}

	auto CoordinateSystemFactory::CreateFlattenedSphere(std::string const& name, double semiMajorAxis, double inverseFlattening, LinearUnit const& linearUnit) -> Ellipsoid
	{
		return Ellipsoid::CreateFromSemiMajorAxisInverseFlattening(name, "", 0, semiMajorAxis, inverseFlattening, linearUnit);
	}

	auto CoordinateSystemFactory::CreatePrimeMeridian(std::string const& name, AngularUnit const& angularUnit, double longitude) -> PrimeMeridian
	{
		return PrimeMeridian{name, "", 0, "", "", angularUnit, longitude};
	}

	auto CoordinateSystemFactory::CreateLocalCoordinateSystem(std::string const& name, std::shared_ptr<ILocalDatum> const& datum, AnyUnit const& unit, std::vector<AxisInfo> const& arExes) -> std::shared_ptr<LocalCoordinateSystem>
	{
		return make_shared<LocalCoordinateSystem>(name, datum, unit, arExes);
	}

	auto CoordinateSystemFactory::CreateLocalDatum(std::string const& name, int datumType) -> std::shared_ptr<ILocalDatum>
	{
		return make_shared<LocalDatum>(name, datumType);
	}

	auto CoordinateSystemFactory::CreateHorizontalDatum(std::string const& name, int datumType, Ellipsoid const& ellipsoid, std::shared_ptr<Wgs84ConversionInfo> const& toWGS84) -> std::shared_ptr<IHorizontalDatum>
	{
		return make_shared<HorizontalDatum>(name, "", 0, "", "", datumType, ellipsoid, toWGS84);
	}

	auto CoordinateSystemFactory::CreateGeographicCoordinateSystem(std::string const& name, AngularUnit const& angularUnit, std::shared_ptr<IHorizontalDatum> const& horizontalDatum, PrimeMeridian const& primeMeridian, AxisInfo axis0, AxisInfo axis1) -> std::shared_ptr<GeographicCoordinateSystem>
	{
		std::vector<AxisInfo> axis;
		axis.resize(2);

		std::vector<AnyUnit> units;
		units.resize(2);

		axis[0] = axis0;
		axis[1] = axis1;
		units[0] = angularUnit;
		units[1] = angularUnit;
		return make_shared<GeographicCoordinateSystem>(name, "", 0, "", "", angularUnit, horizontalDatum, primeMeridian, axis, units);
	}

	auto CoordinateSystemFactory::CreateProjectedCoordinateSystem(std::string const& name, std::shared_ptr<GeographicCoordinateSystem> const& gcs, std::shared_ptr<IProjection> const& projection, LinearUnit const& linearUnit, AxisInfo axis0, AxisInfo axis1) -> std::shared_ptr<ProjectedCoordinateSystem>
	{
		return make_shared<ProjectedCoordinateSystem>(name, "", 0, "", "", gcs, projection, linearUnit, axis0, axis1);
	}

	auto CoordinateSystemFactory::CreateVerticalDatum(std::string const& name, int datumType) -> std::shared_ptr<IVerticalDatum>
	{
		return make_shared<VerticalDatum>(name, "", 0, "", "", datumType);
	}

	auto CoordinateSystemFactory::ModifyWithAxisNorthEast(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem>
	{
		return WithAxesNorthEast(coordinateSystem);
	}

	auto CoordinateSystemFactory::ModifyWithAxisEastNorth(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem>
	{
		return WithAxesEastNorth(coordinateSystem);
	}

	auto CoordinateSystemFactory::CreateCompoundUnknown() const->std::shared_ptr<CompoundCoordinateSystem>
	{
		return CreateUnknownCompound();
	}

	auto CoordinateSystemFactory::WithAxesEastNorth(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem>
	{
		if (coordinateSystem->GetAxis(0).GetOrientation() == AxisOrientationEnum::East &&
			coordinateSystem->GetAxis(1).GetOrientation() == AxisOrientationEnum::North)
			return coordinateSystem;

		if (auto const geographic = dynamic_pointer_cast<GeographicCoordinateSystem>(coordinateSystem); nullptr != geographic)
		{
			std::vector<AxisInfo> axis;
			axis.resize(coordinateSystem->GetDimension());
			axis[0] = AxisInfo("Lon", AxisOrientationEnum::East);
			axis[1] = AxisInfo("Lat", AxisOrientationEnum::North);

			return make_shared<GeographicCoordinateSystem>(
				geographic->GetName(),
				geographic->GetAuthority(),
				geographic->GetAuthorityCode(),
				geographic->GetAlias(),
				geographic->GetRemarks(),
				geographic->GetAngularUnit(),
				dynamic_pointer_cast<IHorizontalDatum>(geographic->GetHorizontalDatum()),
				geographic->GetPrimeMeridian(),
				axis,
				geographic->GetUnits());
		}

		if (auto const projected = dynamic_pointer_cast<ProjectedCoordinateSystem>(coordinateSystem); nullptr != projected)
		{
			return make_shared<ProjectedCoordinateSystem>(
				projected->GetName(),
				projected->GetAuthority(),
				projected->GetAuthorityCode(),
				projected->GetAlias(),
				projected->GetRemarks(),
				dynamic_pointer_cast<GeographicCoordinateSystem>(projected->GetGeographicCoordinateSystem()),
				projected->GetProjection(),
				projected->GetLinearUnit(),
				AxisInfo("X", AxisOrientationEnum::East),
				AxisInfo("Y", AxisOrientationEnum::North));
		}

		return coordinateSystem;
	}

	auto CoordinateSystemFactory::WithAxesNorthEast(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem>
	{
		if (coordinateSystem->GetAxis(0).GetOrientation() == AxisOrientationEnum::North &&
			coordinateSystem->GetAxis(1).GetOrientation() == AxisOrientationEnum::East)
			return coordinateSystem;

		auto const geographic = dynamic_pointer_cast<GeographicCoordinateSystem>(coordinateSystem);
		if (nullptr != geographic)
		{
			std::vector<AxisInfo> axis;
			axis.resize(2);
			axis[0] = AxisInfo("Lat", AxisOrientationEnum::North);
			axis[1] = AxisInfo("Lon", AxisOrientationEnum::East);

			return make_shared<GeographicCoordinateSystem>(
				geographic->GetName(),
				geographic->GetAuthority(),
				geographic->GetAuthorityCode(),
				geographic->GetAlias(),
				geographic->GetRemarks(),
				geographic->GetAngularUnit(),
				dynamic_pointer_cast<IHorizontalDatum>(geographic->GetHorizontalDatum()),
				geographic->GetPrimeMeridian(),
				axis,
				geographic->GetUnits());
		}

		if (auto const projected = dynamic_pointer_cast<ProjectedCoordinateSystem>(coordinateSystem); nullptr != projected)
		{
			return make_shared<ProjectedCoordinateSystem>(
				projected->GetName(),
				projected->GetAuthority(),
				projected->GetAuthorityCode(),
				projected->GetAlias(),
				projected->GetRemarks(),
				dynamic_pointer_cast<GeographicCoordinateSystem>(projected->GetGeographicCoordinateSystem()),
				projected->GetProjection(),
				projected->GetLinearUnit(),
				AxisInfo("Y", AxisOrientationEnum::North),
				AxisInfo("X", AxisOrientationEnum::East));
		}

		return coordinateSystem;
	}

	auto CoordinateSystemFactory::Create2DSystemFor3DSystem(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<CoordinateSystem>
	{
		assert(3 == coordinateSystem->GetDimension());

		if (coordinateSystem->GetAuthority() == "EPSG")
			return GetCoordinateSystemAuthorityFactory()->CreateGeographic2DSystemForGeographic3DSystem(coordinateSystem->GetAuthorityCode(), coordinateSystem->GetAxis(0), coordinateSystem->GetAxis(1));

		return nullptr;
	}

	auto CoordinateSystemFactory::FindGeographicSystem3D(std::shared_ptr<GeographicCoordinateSystem> const& coordinateSystem) -> std::shared_ptr<GeographicCoordinateSystem>
	{
		if (3 == coordinateSystem->GetDimension())
			return coordinateSystem;

		if (coordinateSystem->GetAuthority() == "EPSG")
			return GetCoordinateSystemAuthorityFactory()->FindGeographicSystem3D(coordinateSystem->GetAuthorityCode(), coordinateSystem->GetAxis(0), coordinateSystem->GetAxis(1));

		return nullptr;
	}

	auto CoordinateSystemFactory::CreateUnknownCompound() -> std::shared_ptr<CompoundCoordinateSystem>
	{
		std::string const wkt("COMPD_CS[\"Local or unknown horizontal coordinate system + Local or unknown vertical coordinate system\",LOCAL_CS[\"Local or unknown horizontal coordinate system\", LOCAL_DATUM[\"Local or unknown horizontal datum\", 32767],UNIT[\"unknown\",1.0000000000],AXIS[\"X\", EAST],AXIS[\"Y\",NORTH]],LOCAL_CS[\"Local or unknown vertical coordinate system\", LOCAL_DATUM[\"Local or unknown vertical datum\", 2005],UNIT[\"unknown\",1.0000000000],AXIS[\"Z\",UP]]]");
		Wkt::WktDeserializer const deserializadorWkt(wkt);
		return dynamic_pointer_cast<CompoundCoordinateSystem>(CreateFrom(deserializadorWkt.GetRootNode()));
	}
}
