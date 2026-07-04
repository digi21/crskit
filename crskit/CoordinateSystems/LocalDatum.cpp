#include "pch.h"
#include "BaseInfo.h"
#include "Datum.h"
#include "LocalDatum.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	LocalDatum::LocalDatum(std::string const& _name, int datumType)
		: Datum{_name, "", 0, "", "", datumType}
	{
	}

	LocalDatum::LocalDatum(std::string const& name, int datumType, std::string const& authority, int authorityCode)
		: Datum{name, authority, authorityCode, "", "", datumType}
	{
	}

	auto LocalDatum::FromWkt(Wkt::TokenWkt const& tokenDatum) -> std::shared_ptr<LocalDatum>
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenDatum);

		return std::make_shared<LocalDatum>(
			tokenDatum.GetChild(0).GetString(),
			tokenDatum.GetChild(1).GetInt(),
			authority,
			authorityCode);
	}

#pragma region IInfo members
	auto LocalDatum::GetWkt() const -> std::string
	{
		if (!_authority.empty() && _authorityCode)
		{
			return std::format("LOCAL_DATUM[\"{}\",{},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				GetDatumType(),
				_authority.c_str(),
				_authorityCode);
		}
		else
		{
			return std::format("LOCAL_DATUM[\"{}\",{}]",
				_name.c_str(),
				GetDatumType());
		}
	}
#pragma endregion
}
