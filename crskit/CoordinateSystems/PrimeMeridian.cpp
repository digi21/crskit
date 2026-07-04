#include "pch.h"
#include "PrimeMeridian.h"
#include "CoordinateSystemAuthorityFactory.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	PrimeMeridian::PrimeMeridian(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, AngularUnit const& angularUnit, double longitude)
		: _name{name}
		, _authority{authority}
		, _authorityCode{authorityCode}
		, _alias{alias}
		, _remarks{info}
		, _angularUnit{angularUnit}
		, _longitude{longitude}
	{
	}

	auto PrimeMeridian::FromWkt(Wkt::TokenWkt const& meridianToken) -> PrimeMeridian
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(meridianToken);

		if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
			return GetCoordinateSystemAuthorityFactory()->CreatePrimeMeridian(authorityCode);

		return PrimeMeridian{
			meridianToken.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			GetCoordinateSystemAuthorityFactory()->CreateAngularUnit(MEASURE_UNIT_DEGREES),
			meridianToken.GetChild(1).GetDouble()};
	}

	auto PrimeMeridian::GetWkt() const -> std::string
	{
		if (!_authority.empty() && _authorityCode)
			return std::format("PRIMEM[\"{}\",{:.16g},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				_longitude,
				_authority.c_str(),
				_authorityCode);

		return std::format("PRIMEM[\"{}\",{:.16g}]",
			_name.c_str(),
			_longitude);
	}
}
