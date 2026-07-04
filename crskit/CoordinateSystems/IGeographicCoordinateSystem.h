#pragma once

#include <memory>

#include "AngularUnit.h"
#include "IHorizontalCoordinateSystem.h"
#include "PrimeMeridian.h"

namespace CrsKit::CoordinateSystems
{
	struct IGeographicCoordinateSystem : virtual public IHorizontalCoordinateSystem
	{
		[[nodiscard]] virtual auto GetAngularUnit() const -> AngularUnit = 0;

		[[nodiscard]] virtual auto GetPrimeMeridian() const -> PrimeMeridian = 0;

		[[nodiscard]] virtual auto CompareTo(std::shared_ptr<IGeographicCoordinateSystem> const& obj) const -> int = 0;
	};
}
