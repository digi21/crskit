#pragma once

#include <memory>

#include "ICoordinateSystem.h"
#include "IHorizontalDatum.h"

namespace CrsKit::CoordinateSystems
{
	struct IHorizontalCoordinateSystem : virtual public ICoordinateSystem
	{
		[[nodiscard]] virtual auto GetHorizontalDatum() const -> std::shared_ptr<IHorizontalDatum> = 0;
	};
}
