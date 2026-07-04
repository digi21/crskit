#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../Parameter.h"
#include "IMathTransform.h"

namespace CrsKit::CoordinateTransformations
{
	struct IMathTransformFactory
	{
		[[nodiscard]] virtual auto CreateConcatenatedTransform(std::shared_ptr<IMathTransform> const& transform1, std::shared_ptr<IMathTransform> const& transform2) -> std::shared_ptr<IMathTransform> = 0;
		[[nodiscard]] virtual auto CreateConcatenatedTransform(std::vector<std::shared_ptr<IMathTransform>> const& transformations) -> std::shared_ptr<IMathTransform> = 0;
		[[nodiscard]] virtual auto CreateFromWkt(std::string const& wkt) -> std::shared_ptr<IMathTransform> = 0;
		[[nodiscard]] virtual auto CreateParameterizedTransform(std::string const& classification, std::vector<Parameter> const& parameters) -> std::shared_ptr<IMathTransform> = 0;
		[[nodiscard]] virtual auto CreatePassThroughTransform(int firstAffectedOrdinate, std::shared_ptr<IMathTransform> const& subTransform) -> std::shared_ptr<IMathTransform> = 0;
	};
}
