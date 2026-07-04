#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "IMathTransform.h"

namespace CrsKit::CoordinateTransformations
{
	class _modeCrsKit ConcatenatedTransform
	: public IMathTransform
	{
#pragma region Internal fields
		std::vector<std::shared_ptr<IMathTransform>> _transformations;
		// weak_ptr to avoid a reference cycle: the 3D/compound factory cross-links a direct and an
		// inverse ConcatenatedTransform as each other's inverse; with shared_ptr both (and their whole
		// transform graph) would leak. The owning CoordinateTransformation keeps the real objects alive
		// (CoordinateTransformation::SetInverseMathTransform).
		std::weak_ptr<IMathTransform> _inverseTransformation;

		friend class CoordinateTransformationFactory;

	public:
		ConcatenatedTransform() = default;
		ConcatenatedTransform(std::shared_ptr<IMathTransform> const& transform1, std::shared_ptr<IMathTransform> const& transform2);
		explicit ConcatenatedTransform(std::vector<std::shared_ptr<IMathTransform>> const& transformations);

#pragma endregion
#pragma region IMathTransform members
		auto GetWkt() const -> std::string override;

		auto GetSourceDimension() const -> int override;

		auto GetTargetDimension() const -> int override;

		auto GetIsIdentity() const -> bool override;

		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;

		// Processes the whole point cloud through each stage (one buffer per stage, not per point).
		auto TransformPoints(std::span<double const> sourcePoints) const -> std::vector<double> override;

		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
#pragma region Implementation
		auto GetTransformations() const noexcept -> std::vector<std::shared_ptr<IMathTransform>> const& { return _transformations; }
		void SetInverseTransformation(std::shared_ptr<IMathTransform> const& inverseTransformation) { _inverseTransformation = inverseTransformation;}
#pragma endregion
	};
}
