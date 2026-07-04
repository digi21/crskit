#pragma once

#include <memory>
#include <string>

#include "../CrsKitExport.h"
#include "../CoordinateSystems/CoordinateSystem.h"
#include "ICoordinateTransformation.h"
#include "IMathTransform.h"

namespace CrsKit::CoordinateTransformations
{
	class _modeCrsKit CoordinateTransformation final : public ICoordinateTransformation
	{
#pragma region Private fields
		std::string _name;
		std::string _comentarios;
		std::string _areaOfUse;
		// EPSG coordinate-operation code, or 0 when this transformation is synthetic
		// (axis/unit change, composed projections, user-resolved) and carries no authority code.
		int _authorityCode{};
		std::shared_ptr<CoordinateSystems::CoordinateSystem> _sourceCoordinateSystem;
		std::shared_ptr<CoordinateSystems::CoordinateSystem> _targetCoordinateSystem;
		std::shared_ptr<IMathTransform> _transformation;
		// Optionally keeps a precomputed inverse math-transform alive (3D/compound case) so that
		// GetMathTransform()->GetInverse() returns it without a shared_ptr cycle between the direct
		// and inverse ConcatenatedTransform (which cache each other weakly).
		std::shared_ptr<IMathTransform> _inverseTransformation;
#pragma endregion
#pragma region Internal fields

	public:
		CoordinateTransformation(std::string const& name, std::string const& comentarios, std::string const& areaOfUse, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& sourceCoordinateSystem, std::shared_ptr<CoordinateSystems::CoordinateSystem> const& targetCoordinateSystem, std::shared_ptr<IMathTransform> const& transformation, int authorityCode = 0);

		void SetInverseMathTransform(std::shared_ptr<IMathTransform> const& inverse) { _inverseTransformation = inverse; }
#pragma endregion
#pragma region ICoordinateTransformation members

	public:
		auto GetName() const -> std::string override;

		auto GetAuthority() const -> std::string override;

		auto GetAuthorityCode() const -> std::string override;

		auto GetRemarks() const -> std::string override;

		auto GetAreaOfUse() const -> std::string override;

		auto GetTransformType() const -> TransformType override;

		auto GetSourceCS() const -> std::shared_ptr<CoordinateSystems::ICoordinateSystem> override;

		auto GetTargetCS() const -> std::shared_ptr<CoordinateSystems::ICoordinateSystem> override;

		auto GetMathTransform() const -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
