#pragma once

#include <memory>

#include "ICoordinateSystem.h"
#include "IHorizontalDatum.h"
#include "LinearUnit.h"
#include "PrimeMeridian.h"

namespace CrsKit::CoordinateSystems
{
	struct IGeocentricCoordinateSystem : virtual public ICoordinateSystem
	{
		[[nodiscard]] virtual auto GetHorizontalDatum() const -> std::shared_ptr<IHorizontalDatum> = 0;

		[[nodiscard]] virtual auto GetLinearUnit() const -> LinearUnit = 0;

		[[nodiscard]] virtual auto GetPrimeMeridian() const -> PrimeMeridian = 0;
	};
}
