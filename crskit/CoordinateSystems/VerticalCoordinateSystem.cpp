#include "pch.h"
#include "BaseInfo.h"
#include "CoordinateSystem.h"
#include "VerticalDatum.h"
#include "VerticalCoordinateSystem.h"
#include "LinearUnit.h"
#include "CoordinateSystemAuthorityFactory.h"
#include "Wkt2.h"
#include "../CrsContext.h"
#include "../EsriProjectionEngine.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	VerticalCoordinateSystem::VerticalCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, LinearUnit const& linearUnit, std::shared_ptr<IVerticalDatum> const& datum, AxisInfo const& axis1)
		: CoordinateSystem{name, authority, authorityCode, alias, info, linearUnit, axis1}
		, _verticalUnit{linearUnit}
		, _verticalDatum{datum}
	{
	}

	auto VerticalCoordinateSystem::FromWkt(Wkt::TokenWkt const& tokenVERTCS) -> std::shared_ptr<VerticalCoordinateSystem>
	{
		auto const [authority, authorityCode] = Wkt::ReadAuthority(tokenVERTCS);

		if (!authority.empty())
		{
			if (CatalogWins() && 0 == compareNoCase(authority.c_str(), "EPSG"))
				return GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(authorityCode);
		}
		else if (CatalogWins())
		{
			// We check whether it is of type EsriProjectionEngine
			auto const epsgCode = EsriProjectionEngine::FindVerticalEpsgCodeByName(tokenVERTCS.GetChild(0).GetString());

			if (0 != epsgCode)
				return GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(epsgCode);
		}


		// Position 1 of a GEOGCS must be DATUM
		if (compareNoCase(tokenVERTCS.GetChild(1).GetName().c_str(), "VERT_DATUM"))
			throw WktParseException("VERT_DATUM");

		// Position 2 of a GEOGCS must be UNIT
		if (compareNoCase(tokenVERTCS.GetChild(2).GetName().c_str(), "UNIT"))
			throw WktParseException("UNIT");

		auto axis = AxisInfo::FromWkt(tokenVERTCS);
		if (0 == axis.size())
		{
			axis.resize(1);
			axis[0] = AxisInfo("H", AxisOrientationEnum::Up);
		}

		return make_shared<VerticalCoordinateSystem>(
			tokenVERTCS.GetChild(0).GetString(),
			authority,
			authorityCode,
			"",
			"",
			LinearUnit::FromWkt(tokenVERTCS.GetChild(2)),
			VerticalDatum::FromWkt(tokenVERTCS.GetChild(1)),
			axis[0]);
	}

	auto VerticalCoordinateSystem::GetVerticalUnit() const -> LinearUnit
	{
		return _verticalUnit;
	}

	auto VerticalCoordinateSystem::GetVerticalDatum() const -> std::shared_ptr<IVerticalDatum>
	{
		return _verticalDatum;
	}

	auto VerticalCoordinateSystem::GetWkt(CrsContext const& context) const -> std::string
	{
		auto const version = context.wktVersion;

		if (IsWkt2(version))
			return Wkt::ToWkt2(*this, version);

		std::string text;

		if (!_authority.empty() && 0 != _authorityCode && !IsEsri(version))
		{
			text = std::format("VERT_CS[\"{}\",{},{},{},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				dynamic_pointer_cast<IInfo>(_verticalDatum)->GetWkt().c_str(),
				_verticalUnit.GetWkt().c_str(),
				_axes[0].GetWkt().c_str(),
				_authority.c_str(),
				_authorityCode);
		}
		else
		{
			text = std::format("VERT_CS[\"{}\",{},{},{}]",
				_name.c_str(),
				dynamic_pointer_cast<IInfo>(_verticalDatum)->GetWkt().c_str(),
				_verticalUnit.GetWkt().c_str(),
				_axes[0].GetWkt().c_str());
		}

		if (!_authority.empty() && _authorityCode && IsEsri(version))
			text = EsriProjectionEngine::VerticalWkt(_authorityCode, text, IsEsriWithEpsgCode(version));

		return text;
	}
}
