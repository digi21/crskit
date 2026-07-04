#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "../Parameter.h"
#include "../TokenWkt.h"
#include "IMathTransform.h"
#include "IMathTransformFactory.h"

namespace CrsKit::CoordinateTransformations
{
	class _modeCrsKit MathTransformFactory final
	: public IMathTransformFactory
	{
#pragma region IMathTransformFactory members
	public:
		[[nodiscard]] auto CreateConcatenatedTransform(std::shared_ptr<IMathTransform> const& transform1, std::shared_ptr<IMathTransform> const& transform2) -> std::shared_ptr<IMathTransform> override;
		[[nodiscard]] auto CreateConcatenatedTransform(std::vector<std::shared_ptr<IMathTransform>> const& transformations) -> std::shared_ptr<IMathTransform> override;
		[[nodiscard]] auto CreateFromWkt(std::string const& wkt) -> std::shared_ptr<IMathTransform> override;
		[[nodiscard]] auto CreateParameterizedTransform(std::string const& classification, std::vector<Parameter> const& parameters) -> std::shared_ptr<IMathTransform> override;
		[[nodiscard]] auto CreatePassThroughTransform(int firstAffectedOrdinate, std::shared_ptr<IMathTransform> const& mathTransform) -> std::shared_ptr<IMathTransform> override;
#pragma endregion
#pragma region Implementation
		[[nodiscard]] auto CreateFromWkt(Wkt::TokenWkt const& node, bool inverse) -> std::shared_ptr<IMathTransform>;
		[[nodiscard]] static auto CreateParameterizedTransform(std::string const& classification, std::vector<Parameter> const& parameters, bool inverse) -> std::shared_ptr<IMathTransform>;
#pragma endregion
	};
}
