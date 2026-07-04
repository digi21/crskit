#include "pch.h"
#include "AngularUnit.h"
#include "CoordinateSystemAuthorityFactory.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	AngularUnit::AngularUnit(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, double radiansPerUnit)
		: _name{name}
		, _authority{authority}
		, _authorityCode{authorityCode}
		, _alias{alias}
		, _remarks{info}
		, _factorToRadians{radiansPerUnit}
	{
	}

	AngularUnit::AngularUnit(std::string const& name, double radiansPerUnit)
		: _name{name}
		, _factorToRadians{radiansPerUnit}
	{
	}

	auto AngularUnit::FromWkt(Wkt::TokenWkt const& tokenUnit) -> AngularUnit
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenUnit);

		if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
			return GetCoordinateSystemAuthorityFactory()->CreateAngularUnit(authorityCode);

		return AngularUnit{
			tokenUnit.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			tokenUnit.GetChild(1).GetDouble()};
	}

	auto AngularUnit::GetWkt() const -> std::string
	{
		if (!_authority.empty() && _authorityCode)
			return std::format("UNIT[\"{}\",{:.16g},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				_factorToRadians,
				_authority.c_str(),
				_authorityCode);

		return std::format("UNIT[\"{}\",{:.16g}]", _name.c_str(), _factorToRadians);
	}
}
