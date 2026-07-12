#include "pch.h"
#include "BaseInfo.h"
#include "CoordinateSystem.h"
#include "LocalDatum.h"
#include "LinearUnit.h"
#include "LocalCoordinateSystem.h"
#include "Wkt2.h"

using namespace std;

namespace CrsKit::CoordinateSystems
{
	LocalCoordinateSystem::LocalCoordinateSystem(std::string const& name, std::shared_ptr<ILocalDatum> const& datum, AnyUnit const& linearUnit, std::vector<AxisInfo> const& axis)
		: CoordinateSystem{name, axis, linearUnit}
		, _unit{linearUnit}
		, _datum{datum}
	{
	}

	auto LocalCoordinateSystem::FromWkt(Wkt::TokenWkt const& tokenVERTCS) -> std::shared_ptr<LocalCoordinateSystem>
	{
		// Position 1 of a GEOGCS must be DATUM
		if (compareNoCase(tokenVERTCS.GetChild(1).GetName().c_str(), "LOCAL_DATUM"))
			throw WktParseException("LOCAL_DATUM");

		if (compareNoCase(tokenVERTCS.GetChild(2).GetName().c_str(), "UNIT"))
			throw WktParseException("UNIT");

		auto const axis = AxisInfo::FromWkt(tokenVERTCS);

		return make_shared<LocalCoordinateSystem>(
			tokenVERTCS.GetChild(0).GetString(),
			LocalDatum::FromWkt(tokenVERTCS.GetChild(1)),
			LinearUnit::FromWkt(tokenVERTCS.GetChild(2)),
			axis);
	}

	auto LocalCoordinateSystem::GetLocalDatum() const -> std::shared_ptr<ILocalDatum>
	{
		return _datum;
	}

	auto LocalCoordinateSystem::GetWkt(CrsContext const& context) const -> std::string
	{
		auto const version = context.wktVersion;

		if (IsWkt2(version))
			return Wkt::ToWkt2(*this, version);

		auto const axesString = std::accumulate(_axes.begin(), _axes.end(), std::string{},
			[](std::string const& acc, auto const& axis) { return acc + "," + axis.GetWkt(); });

		if (!_authority.empty() && _authorityCode)
		{
			return std::format("LOCAL_CS[\"{}\",{},{}{},AUTHORITY[\"{}\",\"{}\"]]",
				_name.c_str(),
				dynamic_pointer_cast<IInfo>(_datum)->GetWkt().c_str(),
				WktOf(_unit).c_str(),
				axesString.c_str(),
				_authority.c_str(),
				_authorityCode);
		}

		return std::format("LOCAL_CS[\"{}\",{},{}{}]",
			_name.c_str(),
			dynamic_pointer_cast<IInfo>(_datum)->GetWkt().c_str(),
			WktOf(_unit).c_str(),
			axesString.c_str());
	}
}
