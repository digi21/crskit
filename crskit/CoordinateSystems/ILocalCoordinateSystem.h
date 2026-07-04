#pragma once

#include <memory>

#include "ICoordinateSystem.h"
#include "ILocalDatum.h"

namespace CrsKit::CoordinateSystems
{
	struct ILocalCoordinateSystem : virtual public ICoordinateSystem
	{
		[[nodiscard]] virtual auto GetLocalDatum() const -> std::shared_ptr<ILocalDatum> = 0;
	};
}
