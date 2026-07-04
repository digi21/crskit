#include "pch.h"
#include "BaseInfo.h"
#include "Datum.h"
#include "VerticalDatum.h"
#include "CoordinateSystemAuthorityFactory.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	VerticalDatum::VerticalDatum(std::string const& name, std::string const& authority, int code, std::string const& alias, std::string const& info, int datumType)
		: Datum{name, authority, code, alias, info, datumType}
	{
	}

	auto VerticalDatum::FromWkt(Wkt::TokenWkt const& tokenDatum) -> std::shared_ptr<VerticalDatum>
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenDatum);

		if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
			return dynamic_pointer_cast<VerticalDatum>(GetCoordinateSystemAuthorityFactory()->CreateVerticalDatum(authorityCode));

		return make_shared<VerticalDatum>(
			tokenDatum.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			tokenDatum.GetChild(1).GetInt());
	}

#pragma region IInfo members
	auto VerticalDatum::GetWkt() const -> std::string
	{
		if (!_authority.empty() && _authorityCode)
		{
			return std::format("VERT_DATUM[\"{}\",{},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				GetDatumType(),
				_authority.c_str(),
				_authorityCode);
		}

		return std::format("VERT_DATUM[\"{}\",{}]",
			_name.c_str(),
			GetDatumType());
	}
#pragma endregion
}
