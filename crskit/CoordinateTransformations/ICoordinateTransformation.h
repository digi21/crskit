#pragma once

#include <memory>
#include <string>

#include "../CoordinateSystems/ICoordinateSystem.h"
#include "IMathTransform.h"
#include "TransformType.h"

namespace CrsKit::CoordinateTransformations
{
	struct ICoordinateTransformation
	{
		virtual ~ICoordinateTransformation() = default;

		[[nodiscard]] virtual auto GetName() const -> std::string = 0;
		[[nodiscard]] virtual auto GetAuthority() const -> std::string = 0;
		[[nodiscard]] virtual auto GetAuthorityCode() const -> std::string = 0;
		[[nodiscard]] virtual auto GetRemarks() const -> std::string = 0;
		[[nodiscard]] virtual auto GetAreaOfUse() const -> std::string = 0;
		[[nodiscard]] virtual auto GetTransformType() const -> TransformType = 0;
		[[nodiscard]] virtual auto GetSourceCS() const -> std::shared_ptr<CoordinateSystems::ICoordinateSystem> = 0;
		[[nodiscard]] virtual auto GetTargetCS() const -> std::shared_ptr<CoordinateSystems::ICoordinateSystem> = 0;
		[[nodiscard]] virtual auto GetMathTransform() const -> std::shared_ptr<IMathTransform> = 0;
	};
}
