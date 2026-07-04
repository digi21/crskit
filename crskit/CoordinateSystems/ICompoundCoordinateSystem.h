#pragma once

#include <memory>

#include "ICoordinateSystem.h"

namespace CrsKit::CoordinateSystems
{
	struct ICompoundCoordinateSystem : virtual public ICoordinateSystem
	{
		[[nodiscard]] virtual auto GetHeadCS() const -> std::shared_ptr<ICoordinateSystem> = 0;

		[[nodiscard]] virtual auto GetTailCS() const -> std::shared_ptr<ICoordinateSystem> = 0;
	};
}
