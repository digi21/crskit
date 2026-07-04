#pragma once

#include <vector>

#include "../Positioning/Envelope.h"
#include "AxisInfo.h"
#include "IInfo.h"
#include "Unit.h"

namespace CrsKit::CoordinateSystems
{
	struct ICoordinateSystem : virtual public IInfo
	{
		[[nodiscard]] virtual auto GetDimension() const -> int = 0;

		[[nodiscard]] virtual auto GetDefaultEnvelope() const -> Positioning::Envelope = 0;

		[[nodiscard]] virtual auto GetAxis(int dimension) const -> AxisInfo = 0;

		[[nodiscard]] virtual auto GetUnits(int dimension) const -> AnyUnit = 0;

		[[nodiscard]] virtual auto GetUnits() const -> std::vector<AnyUnit> const& = 0;
	};
}
