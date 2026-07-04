#pragma once

#include <memory>

#include "Ellipsoid.h"

namespace CrsKit::CoordinateSystems
{
	struct IHorizontalDatum
	{
		[[nodiscard]] virtual auto GetEllipsoid() const -> Ellipsoid = 0;

		virtual auto CompareTo(std::shared_ptr<IHorizontalDatum> const& obj) const -> int = 0;
	};
}
