#include "pch.h"
#include "LinearUnit.h"
#include "CoordinateSystemAuthorityFactory.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	LinearUnit::LinearUnit(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, double metersPerUnit)
		: _name{name}
		, _authority{authority}
		, _authorityCode{authorityCode}
		, _alias{alias}
		, _remarks{info}
		, _factorToMeters{metersPerUnit}
	{
	}

	LinearUnit::LinearUnit(std::string const& name, double metersPerUnit)
		: _name{name}
		, _factorToMeters{metersPerUnit}
	{
	}

	auto LinearUnit::FromWkt(Wkt::TokenWkt const& tokenUnit) -> LinearUnit
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenUnit);

		if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
			return GetCoordinateSystemAuthorityFactory()->CreateLinearUnit(authorityCode);

		return LinearUnit{
			tokenUnit.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			tokenUnit.GetChild(1).GetDouble()};
	}

	auto LinearUnit::GetWkt() const -> std::string
	{
		if (!_authority.empty() && _authorityCode)
			return std::format("UNIT[\"{}\",{:.16g},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				_factorToMeters,
				_authority.c_str(),
				_authorityCode);

		return std::format("UNIT[\"{}\",{:.16g}]",
			_name.c_str(),
			_factorToMeters);
	}
}
