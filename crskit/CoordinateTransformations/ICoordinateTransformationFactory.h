#pragma once

#include <memory>

#include "../CoordinateSystems/CoordinateSystem.h"
#include "../CoordinateTransformationOptions.h"
#include "ICoordinateTransformation.h"

namespace CrsKit::CoordinateTransformations
{
	struct ICoordinateTransformationFactory
	{
		[[nodiscard]] virtual auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS) const -> std::shared_ptr<ICoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const->std::shared_ptr<ICoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateFromVerticalCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const -> std::shared_ptr<ICoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateFrom3DCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCS, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCS, CoordinateTransformationOptions const& options) const -> std::shared_ptr<ICoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateFromHorizontalCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& geographic3D, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target, CoordinateTransformationOptions const& options) const -> std::shared_ptr<ICoordinateTransformation> = 0;
	};
}
