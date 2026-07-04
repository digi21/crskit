#include "pch.h"
#include "BaseInfo.h"
#include "Ellipsoid.h"
#include "DatumType.h"
#include "Datum.h"
#include "HorizontalDatum.h"
#include "CoordinateSystemAuthorityFactory.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	HorizontalDatum::HorizontalDatum(std::string const& name, std::string const& authority, int code, std::string const& alias, std::string const& info, int datumType, Ellipsoid const& ellipsoid, std::shared_ptr<Wgs84ConversionInfo> const& toWgs84)
		: Datum{name, authority, code, alias, info, datumType}
		, _ellipsoid{ellipsoid}
		, _toWgs84{toWgs84}
	{
	}

	auto HorizontalDatum::FromWkt(Wkt::TokenWkt const& tokenDatum) -> std::shared_ptr<HorizontalDatum>
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenDatum);

		if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
			return dynamic_pointer_cast<HorizontalDatum>(GetCoordinateSystemAuthorityFactory()->CreateHorizontalDatum(authorityCode));

		// Position 1 of a DATUM must be SPHEROID or ELLIPSOID (they are synonyms)
		if (compareNoCase(tokenDatum.GetChild(1).GetName().c_str(), "SPHEROID") && compareNoCase(tokenDatum.GetChild(1).GetName().c_str(), "ELLIPSOID"))
			throw WktParseException("SPHEROID or ELLIPSOID not found");

		// Optional Bursa-Wolf parameters to WGS 84: TOWGS84[dx,dy,dz,ex,ey,ez,ppm].
		std::shared_ptr<Wgs84ConversionInfo> toWgs84;
		if (tokenDatum.HasChild("TOWGS84"))
		{
			auto const& node = tokenDatum.GetChild("TOWGS84");
			toWgs84 = std::make_shared<Wgs84ConversionInfo>(
				node.GetChild(0).GetDouble(),
				node.GetChild(1).GetDouble(),
				node.GetChild(2).GetDouble(),
				node.GetChild(3).GetDouble(),
				node.GetChild(4).GetDouble(),
				node.GetChild(5).GetDouble(),
				node.GetChild(6).GetDouble());
		}

		return make_shared<HorizontalDatum>(
			tokenDatum.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			DatumType::HD_Geocentric,
			Ellipsoid::FromWkt(tokenDatum.GetChild(1)),
			toWgs84);
	}

#pragma region IHorizontalDatum members
	auto HorizontalDatum::GetEllipsoid() const -> Ellipsoid
	{
		return _ellipsoid;
	}

#pragma endregion
#pragma region IInfo members
	auto HorizontalDatum::GetWkt() const -> std::string
	{
		// OGC WKT 1 order: DATUM["name", SPHEROID[...], TOWGS84[...]?, AUTHORITY[...]?].
		std::string wkt = std::format("DATUM[\"{}\",{}", _name, _ellipsoid.GetWkt());

		if (_toWgs84)
			wkt += std::format(",TOWGS84[{},{},{},{},{},{},{}]",
				_toWgs84->GetDx(), _toWgs84->GetDy(), _toWgs84->GetDz(),
				_toWgs84->GetEx(), _toWgs84->GetEy(), _toWgs84->GetEz(), _toWgs84->GetPpm());

		if (!_authority.empty() && _authorityCode)
			wkt += std::format(",AUTHORITY[\"{}\",\"{}\"]", _authority, _authorityCode);

		wkt += "]";
		return wkt;
	}
#pragma endregion
#pragma region IComparable<IHorizontalDatum*> members
	auto HorizontalDatum::CompareTo(std::shared_ptr<IHorizontalDatum> const& obj) const -> int
	{
		auto const info = dynamic_pointer_cast<IInfo>(obj);

		if (GetAuthorityCode() != UnknownAuthorityCode)
		{
			if (GetAuthority() != info->GetAuthority())
				return compareNoCase(GetAuthority().c_str(), info->GetAuthority().c_str());

			if (GetAuthorityCode() != info->GetAuthorityCode())
				return GetAuthorityCode() - info->GetAuthorityCode();

			return 0;
		}

		return compareNoCase(GetName().c_str(), info->GetName().c_str());
	}
#pragma endregion
}
