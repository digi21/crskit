#include "pch.h"
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
			// BUT a datum carrying explicit TOWGS84 parameters is taken at face value: snapping it to a
			// same-named catalogue entry would drop the Bursa-Wolf parameters the caller supplied, so we
			// keep the WKT definition as-is.
			if (!tokenGEOGCS.GetChild(1).HasChild("TOWGS84"))
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
