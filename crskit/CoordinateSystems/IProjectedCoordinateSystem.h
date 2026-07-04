#pragma once

#include <memory>

#include "IGeographicCoordinateSystem.h"
#include "IHorizontalCoordinateSystem.h"
#include "IProjection.h"
#include "LinearUnit.h"

namespace CrsKit::CoordinateSystems
{
	struct IProjectedCoordinateSystem : virtual public IHorizontalCoordinateSystem
	{
		[[nodiscard]] virtual auto GetGeographicCoordinateSystem() const -> std::shared_ptr<IGeographicCoordinateSystem> = 0;

		[[nodiscard]] virtual auto GetLinearUnit() const -> LinearUnit = 0;

		[[nodiscard]] virtual auto GetProjection() const -> std::shared_ptr<IProjection> = 0;
	};
}
