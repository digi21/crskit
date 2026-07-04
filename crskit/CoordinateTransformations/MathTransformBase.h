#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../CoordinateSystems/IProjection.h"
#include "IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class _modeCrsKit MathTransformBase : public IMathTransform
	{
	protected:
		std::shared_ptr<CoordinateSystems::IProjection> _parameters;
		bool _inverse;
		std::shared_ptr<IMathTransform> _inverseTransformation;

	protected:
		explicit MathTransformBase(bool inverse);
		MathTransformBase(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		explicit MathTransformBase(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);

#pragma region IMathTransform members

	public:
		auto GetWkt() const -> std::string override;
		auto GetSourceDimension() const -> int override;
		auto GetTargetDimension() const -> int override;
		auto GetIsIdentity() const -> bool override;
#pragma endregion
	};
}
