#include "pch.h"
#include "BaseInfo.h"
#include "CoordinateSystem.h"
#include "HorizontalDatum.h"
#include "LinearUnit.h"
#include "PrimeMeridian.h"
#include "GeocentricCoordinateSystem.h"
#include "CoordinateSystemAuthorityFactory.h"
#include "Wkt2.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	GeocentricCoordinateSystem::GeocentricCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, LinearUnit const& linearUnit, std::shared_ptr<IHorizontalDatum> const& horizontalDatum, PrimeMeridian const& primeMeridian, AxisInfo const& axis1, AxisInfo const& axis2, AxisInfo const& axis3)
		: CoordinateSystem{name, authority, authorityCode, alias, info, linearUnit, axis1, axis2, axis3}
		, _datum{horizontalDatum}
		, _linearUnit{linearUnit}
		, _primeMeridian{primeMeridian}
	{
	}

	auto GeocentricCoordinateSystem::FromWkt(Wkt::TokenWkt const& tokenGEOCCS) -> std::shared_ptr<GeocentricCoordinateSystem>
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenGEOCCS);

		if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
			return dynamic_pointer_cast<GeocentricCoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(authorityCode));

		// Position 1 of a GEOGCS must be DATUM
		if (compareNoCase(tokenGEOCCS.GetChild(1).GetName().c_str(), "DATUM"))
			throw WktParseException("DATUM");

		// Position 2 of a GEOGCS must be PRIMEM
		if (compareNoCase(tokenGEOCCS.GetChild(2).GetName().c_str(), "PRIMEM"))
			throw WktParseException("PRIMEM");

		// Position 3 of a GEOGCS must be UNIT
		if (compareNoCase(tokenGEOCCS.GetChild(3).GetName().c_str(), "UNIT"))
			throw WktParseException("UNIT");

		auto const axis = AxisInfo::FromWkt(tokenGEOCCS);

		return make_shared<GeocentricCoordinateSystem>(
			tokenGEOCCS.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			LinearUnit::FromWkt(tokenGEOCCS.GetChild(3)),
			HorizontalDatum::FromWkt(tokenGEOCCS.GetChild(1)),
			PrimeMeridian::FromWkt(tokenGEOCCS.GetChild(2)),
			axis[0],
			axis[1],
			axis[2]);
	}

	auto GeocentricCoordinateSystem::GetHorizontalDatum() const -> std::shared_ptr<IHorizontalDatum>
	{
		return _datum;
	}

	auto GeocentricCoordinateSystem::GetLinearUnit() const -> LinearUnit
	{
		return _linearUnit;
	}

	auto GeocentricCoordinateSystem::GetPrimeMeridian() const -> PrimeMeridian
	{
		return _primeMeridian;
	}

	auto GeocentricCoordinateSystem::GetWkt(CrsContext const& context) const -> std::string
	{
		auto const version = context.wktVersion;

		if (IsWkt2(version))
			return Wkt::ToWkt2(*this, version);

		if (!_authority.empty() && _authorityCode)
		{
			return std::format("GEOCCS[\"{}\",{},{},{},{},{},AUTHORITY[\"{}\",\"{}\"]]",
						_name.c_str(),
						dynamic_pointer_cast<IInfo>(_datum)->GetWkt().c_str(),
						_primeMeridian.GetWkt().c_str(),
						_linearUnit.GetWkt().c_str(),
						_axes[0].GetWkt().c_str(),
						_axes[1].GetWkt().c_str(),
						_authority.c_str(),
						_authorityCode);
		}

		return std::format("GEOCCS[\"{}\",{},{},{},{},{}]",
						_name.c_str(),
						dynamic_pointer_cast<IInfo>(_datum)->GetWkt().c_str(),
						_primeMeridian.GetWkt().c_str(),
						_linearUnit.GetWkt().c_str(),
						_axes[0].GetWkt().c_str(),
						_axes[1].GetWkt().c_str());
	}
}
