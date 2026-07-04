#pragma once

#include <memory>

#include "ICoordinateSystem.h"
#include "IVerticalDatum.h"
#include "LinearUnit.h"

namespace CrsKit::CoordinateSystems
{
	struct IVerticalCoordinateSystem : virtual public ICoordinateSystem
	{
		[[nodiscard]] virtual auto GetVerticalDatum() const -> std::shared_ptr<IVerticalDatum> = 0;

		[[nodiscard]] virtual auto GetVerticalUnit() const -> LinearUnit = 0;
	};
}
