#include "pch.h"
#include <numbers>
#include "BaseInfo.h"
#include "CoordinateSystem.h"
#include "HorizontalDatum.h"
#include "HorizontalCoordinateSystem.h"
#include "AngularUnit.h"
#include "PrimeMeridian.h"
#include "GeographicCoordinateSystem.h"
#include "CoordinateSystemAuthorityFactory.h"
#include "Wkt2.h"
#include "../CrsContext.h"
#include "../EsriProjectionEngine.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	GeographicCoordinateSystem::GeographicCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AngularUnit const& angularUnit, std::shared_ptr<IHorizontalDatum> const& datum, PrimeMeridian const& primeMeridian, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units)
		: HorizontalCoordinateSystem{name, authority, authorityCode, alias, info, axis, units, datum}
		, _angularUnit{angularUnit}
		, _primeMeridian{primeMeridian}
	{
	}

	auto GeographicCoordinateSystem::FromWkt(Wkt::TokenWkt const& tokenGEOGCS) -> std::shared_ptr<GeographicCoordinateSystem>
	{
		if (auto const alias = FindCrsAlias(tokenGEOGCS))
		{
			auto const axis = AxisInfo::FromWkt(tokenGEOGCS);
			if (axis.empty())
				return GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(*alias);
			return GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(*alias, axis);
		}

		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenGEOGCS);

		if (!authority.empty())
		{
			if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
			{
				auto const axis = AxisInfo::FromWkt(tokenGEOGCS);

				if (axis.empty())
					return GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(authorityCode);

				return GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(authorityCode, axis);
			}
		}
		else if (CatalogWins())
		{
			// No AUTHORITY node: we try to recognise the CRS by name and use the catalogue version.
			// A datum carrying explicit TOWGS84 parameters is kept as-is, because snapping drops them,
			// unless CrsContext::snapEsriNameWithTowgs84 is set and MatchesCatalogueDefinition holds.
			auto const hasTowgs84 = tokenGEOGCS.GetChild(1).HasChild("TOWGS84");
			if (!hasTowgs84)
			{
				// We check whether it is of type EsriProjectionEngine
				auto const epsgCode = EsriProjectionEngine::FindHorizontalEpsgCodeByName(tokenGEOGCS.GetChild(0).GetString());

				if (0 != epsgCode)
				{
					auto const axis = AxisInfo::FromWkt(tokenGEOGCS);

					if (axis.empty())
						return GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(epsgCode);

					return GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(epsgCode, axis);
				}
			}
			else if (GetDefaultContext()->snapEsriNameWithTowgs84)
			{
				if (auto system = SnapEsriNameWithTowgs84(tokenGEOGCS))
					return system;
			}
		}

		// Position 1 of a GEOGCS must be DATUM
		if (compareNoCase(tokenGEOGCS.GetChild(1).GetName().c_str(), "DATUM"))
			throw WktParseException("DATUM");

		// Position 2 of a GEOGCS must be PRIMEM
		if (compareNoCase(tokenGEOGCS.GetChild(2).GetName().c_str(), "PRIMEM"))
			throw WktParseException("PRIMEM");

		// Position 3 of a GEOGCS must be UNIT
		if (compareNoCase(tokenGEOGCS.GetChild(3).GetName().c_str(), "UNIT"))
			throw WktParseException("UNIT");

		auto axis = AxisInfo::FromWkt(tokenGEOGCS);
		if (0 == axis.size())
		{
			axis.resize(2);
			axis[0] = AxisInfo("Lon", AxisOrientationEnum::East);
			axis[1] = AxisInfo("Lat", AxisOrientationEnum::North);
		}

		std::vector<AnyUnit> units;
		if (axis.size() == 2)
		{
			units.resize(2);
			units[0] = AngularUnit::FromWkt(tokenGEOGCS.GetChild(3));
			units[1] = units[0];
		}
		else
		{
			units.resize(3);
			units[0] = AngularUnit::FromWkt(tokenGEOGCS.GetChild(3));
			units[1] = units[0];
			units[2] = GetCoordinateSystemAuthorityFactory()->CreateLinearUnit(9001);
		}

		return make_shared<GeographicCoordinateSystem>(
			tokenGEOGCS.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			AngularUnit::FromWkt(tokenGEOGCS.GetChild(3)),
			HorizontalDatum::FromWkt(tokenGEOGCS.GetChild(1)),
			PrimeMeridian::FromWkt(tokenGEOGCS.GetChild(2)),
			axis,
			units);
	}

	auto GeographicCoordinateSystem::SnapEsriNameWithTowgs84(Wkt::TokenWkt const& tokenGEOGCS) -> std::shared_ptr<GeographicCoordinateSystem>
	{
		auto const epsgCode = EsriProjectionEngine::FindHorizontalEpsgCodeByName(tokenGEOGCS.GetChild(0).GetString());
		if (0 == epsgCode)
			return {};

		try
		{
			auto const axis = AxisInfo::FromWkt(tokenGEOGCS);
			auto system = axis.empty()
				? GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(epsgCode)
				: GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(epsgCode, axis);
			return MatchesCatalogueDefinition(tokenGEOGCS, *system) ? system : nullptr;
		}
		catch (std::exception const&)
		{
			// The ESRI name belongs to a CRS of another type (a projected CRS, for instance): build from the text.
			return {};
		}
	}

	auto GeographicCoordinateSystem::MatchesCatalogueDefinition(Wkt::TokenWkt const& tokenGEOGCS, IGeographicCoordinateSystem const& system) -> bool
	{
		if (tokenGEOGCS.GetChildCount() < 4)
			return false;

		auto const& datum = tokenGEOGCS.GetChild(1);
		auto const& meridian = tokenGEOGCS.GetChild(2);
		if (datum.GetChildCount() < 2 || compareNoCase(datum.GetChild(1).GetName().c_str(), "SPHEROID") || compareNoCase(meridian.GetName().c_str(), "PRIMEM"))
			return false;

		auto const& spheroid = datum.GetChild(1);
		if (spheroid.GetChildCount() < 3 || meridian.GetChildCount() < 2)
			return false;

		// The semi-major axis is in metres. The prime meridian is read in degrees, as PrimeMeridian::FromWkt reads
		// it and as ESRI writes it even when the GEOGCS unit is the grad.
		auto const semiMajorAxis = spheroid.GetChild(1).GetDouble();
		auto const inverseFlattening = spheroid.GetChild(2).GetDouble();
		auto const meridianRadians = meridian.GetChild(1).GetDouble() * std::numbers::pi / 180.0;
		auto const radiansPerUnit = AngularUnit::FromWkt(tokenGEOGCS.GetChild(3)).GetRadiansPerUnit();

		auto const& ellipsoid = system.GetHorizontalDatum()->GetEllipsoid();
		auto const systemMeridian = system.GetPrimeMeridian();
		auto const systemRadiansPerUnit = system.GetAngularUnit().GetRadiansPerUnit();

		return fabs(ellipsoid.GetSemiMajorAxis() * ellipsoid.GetLinearUnit().GetMetersPerUnit() - semiMajorAxis) <= 1e-2
			&& fabs(ellipsoid.GetInverseFlattening() - inverseFlattening) <= 1e-4
			&& fabs(systemMeridian.GetLongitude() * systemMeridian.GetAngularUnit().GetRadiansPerUnit() - meridianRadians) <= 1e-8
			&& fabs(systemRadiansPerUnit - radiansPerUnit) <= 1e-9 * systemRadiansPerUnit;
	}

	auto GeographicCoordinateSystem::GetAngularUnit() const -> AngularUnit
	{
		return _angularUnit;
	}

	auto GeographicCoordinateSystem::GetPrimeMeridian() const -> PrimeMeridian
	{
		return _primeMeridian;
	}

	auto GeographicCoordinateSystem::GetWkt(CrsContext const& context) const -> std::string
	{
		auto const version = context.wktVersion;

		if (IsWkt2(version))
			return Wkt::ToWkt2(*this, version);

		std::string text;

		if (!_authority.empty() && _authorityCode)
		{
			if (2 == _axes.size())
			{
				return std::format("GEOGCS[\"{}\",{},{},{},{},{},AUTHORITY[\"{}\",\"{}\"]]",
							_name.c_str(),
							dynamic_pointer_cast<IInfo>(_horizontalDatum)->GetWkt().c_str(),
							_primeMeridian.GetWkt().c_str(),
							_angularUnit.GetWkt().c_str(),
							_axes[0].GetWkt().c_str(),
							_axes[1].GetWkt().c_str(),
							_authority.c_str(),
							_authorityCode);
			}
			else if (3 == _axes.size())
			{
				return std::format("GEOGCS[\"{}\",{},{},{},{},{},{},AUTHORITY[\"{}\",\"{}\"]]",
							_name.c_str(),
							dynamic_pointer_cast<IInfo>(_horizontalDatum)->GetWkt().c_str(),
							_primeMeridian.GetWkt().c_str(),
							_angularUnit.GetWkt().c_str(),
							_axes[0].GetWkt().c_str(),
							_axes[1].GetWkt().c_str(),
							_axes[2].GetWkt().c_str(),
							_authority.c_str(),
							_authorityCode);
			}
		}
		else
		{
			if (2 == _axes.size())
			{
				return std::format("GEOGCS[\"{}\",{},{},{},{},{}]",
							_name.c_str(),
							dynamic_pointer_cast<IInfo>(_horizontalDatum)->GetWkt().c_str(),
							_primeMeridian.GetWkt().c_str(),
							_angularUnit.GetWkt().c_str(),
							_axes[0].GetWkt().c_str(),
							_axes[1].GetWkt().c_str());
			}
			else if (3 == _axes.size())
			{
				return std::format("GEOGCS[\"{}\",{},{},{},{},{},{}]",
							_name.c_str(),
							dynamic_pointer_cast<IInfo>(_horizontalDatum)->GetWkt().c_str(),
							_primeMeridian.GetWkt().c_str(),
							_angularUnit.GetWkt().c_str(),
							_axes[0].GetWkt().c_str(),
							_axes[1].GetWkt().c_str(),
				_axes[2].GetWkt().c_str());
			}
		}

		return text;
	}

	auto GeographicCoordinateSystem::CompareTo(std::shared_ptr<IGeographicCoordinateSystem> const& obj) const -> int
	{
		if (obj.get() == static_cast<IGeographicCoordinateSystem const*>(this))
			return 0;

		auto value = compareNoCase(GetAuthority().c_str(), obj->GetAuthority().c_str());
		if (0 != value)
			return value;

		value = GetAuthorityCode() - obj->GetAuthorityCode();
		if (0 != value)
			return value;

		// Two systems with no authority code (e.g. parsed from bare WKT) are not yet told apart by the
		// lines above, so we compare their datums. Without this, distinct datums such as ED50 and WGS 84
		// would compare equal whenever their axes happen to match, which silently skips datum shifts.
		if (auto const datum = GetHorizontalDatum())
		{
			value = datum->CompareTo(obj->GetHorizontalDatum());
			if (0 != value)
				return value;
		}

		value = GetAxis(0).CompareTo(obj->GetAxis(0));
		if (0 != value)
			return value;

		return GetAxis(1).CompareTo(obj->GetAxis(1));
	}
}
