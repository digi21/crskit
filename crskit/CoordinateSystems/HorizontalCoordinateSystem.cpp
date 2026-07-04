#include "pch.h"
#include "CoordinateSystem.h"
#include "HorizontalCoordinateSystem.h"

namespace CrsKit::CoordinateSystems
{
	HorizontalCoordinateSystem::HorizontalCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units, Positioning::Envelope const& areaUso, std::shared_ptr<IHorizontalDatum> const& datum)
		: CoordinateSystem{name, authority, authorityCode, alias, info, axis, units, areaUso}
		, _horizontalDatum{datum}
	{
	}

	HorizontalCoordinateSystem::HorizontalCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units, std::shared_ptr<IHorizontalDatum> const& datum)
		: CoordinateSystem{name, authority, authorityCode, alias, info, axis, units}
		, _horizontalDatum{datum}
	{
	}

	HorizontalCoordinateSystem::HorizontalCoordinateSystem(std::string const& name, std::shared_ptr<IHorizontalDatum> const& datum, std::vector<AxisInfo> const& axis, std::vector<AnyUnit> const& units)
		: CoordinateSystem{name, axis, units}
		, _horizontalDatum{datum}
	{
	}

	HorizontalCoordinateSystem::HorizontalCoordinateSystem(std::string const& name, std::string const& authority, int authorityCode, std::string const& alias, std::string const& info, std::shared_ptr<IHorizontalDatum> const& datum, AnyUnit const& unit, AxisInfo const& axis1, AxisInfo const& axis2)
		: CoordinateSystem{name, authority, authorityCode, alias, info, unit, axis1, axis2}
		, _horizontalDatum{datum}
	{
	}

	auto HorizontalCoordinateSystem::GetHorizontalDatum() const -> std::shared_ptr<IHorizontalDatum>
	{
		return _horizontalDatum;
	}
}
