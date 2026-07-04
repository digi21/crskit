#pragma once

#include <memory>
#include <string>

#include "../CoordinateSystems/CoordinateSystem.h"
#include "../CoordinateSystems/GeographicCoordinateSystem.h"
#include "../CoordinateSystems/HorizontalCoordinateSystem.h"
#include "../CoordinateSystems/VerticalCoordinateSystem.h"
#include "../CoordinateTransformationOptions.h"
#include "CoordinateTransformation.h"
#include "ICoordinateTransformation.h"

namespace CrsKit::CoordinateTransformations
{
	struct ICoordinateTransformationAuthorityFactory
	{
		virtual ~ICoordinateTransformationAuthorityFactory() = default;
		[[nodiscard]] virtual auto GetAuthority() const -> std::string = 0;
		[[nodiscard]] virtual auto CreateFromTransformationCode(int code) -> std::shared_ptr<ICoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateFromCoordinateSystemsCodes(int sourceCode, int targetCode) -> std::shared_ptr<ICoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateFromCoordinateSystemsCodes(int sourceCode, int targetCode, CoordinateTransformationOptions const& options) -> std::shared_ptr<ICoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target) const->std::shared_ptr<CoordinateTransformations::ICoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateFromCoordinateSystems(std::shared_ptr<CoordinateSystems::CoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& target, CoordinateTransformationOptions const& options) const->std::shared_ptr<CoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateCoordinateTransformBetweenHorizontalSystems(std::shared_ptr<CoordinateSystems::HorizontalCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::HorizontalCoordinateSystem> const& target, CoordinateTransformationOptions const& options) const->std::shared_ptr<CoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateCoordinateTransformBetweenGeographicVerticalSystems(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& target, bool inverse, CoordinateTransformationOptions const& options) const->std::shared_ptr<CoordinateTransformations::CoordinateTransformation> = 0;
		[[nodiscard]] virtual auto CreateCoordinateTransformBetweenGeographicVerticalSystems(std::shared_ptr<CoordinateSystems::GeographicCoordinateSystem> const& source, std::shared_ptr<CoordinateSystems::VerticalCoordinateSystem> const& target, int operationCode, bool inverse) const->std::shared_ptr<CoordinateTransformation> = 0;
	};
}
