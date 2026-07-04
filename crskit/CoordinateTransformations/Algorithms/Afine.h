#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../../Mathematics/Matrix.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class Affine : public IMathTransform
	{

#pragma region Private fields

	private:
		CrsKit::Math::Matrix _matriz;
		std::shared_ptr<IMathTransform> _inverseTransformation;
		int _rows;
		int _columns;
		bool _inverse;
#pragma endregion
#pragma region Constructors

	public:
		Affine() = default;
		Affine(int rows, int columns, Math::Matrix const& matriz, bool inverse);
		Affine(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

		static auto IdentityTransform() -> std::shared_ptr<Affine>;
		static auto TransformXyzToZxy() -> std::shared_ptr<Affine>;
		static auto TransformZxyToXyz() -> std::shared_ptr<Affine>;
		static auto TransformXyzToYxz() -> std::shared_ptr<Affine>;
		static auto TransformXyzToXyXyz() -> std::shared_ptr<Affine>;
		static auto AffineMainDiagonalTransform(double value, unsigned int componentes) -> std::shared_ptr<Affine>;

		static auto AffineSwapAxesTransform(unsigned int componentes) -> std::shared_ptr<Affine>;
		static auto CreateAffine2DTo3DParameters() -> std::shared_ptr<CoordinateSystems::IProjection>;
		static auto Affine2DTo3DTransform() -> std::shared_ptr<Affine>;
		static auto Affine2DTo3DSwapAxesTransform() -> std::shared_ptr<Affine>;
		static auto CreateAffine3DTo2DParameters() -> std::shared_ptr<CoordinateSystems::IProjection>;
		static auto Affine3DTo2DTransform() -> std::shared_ptr<Affine>;
		static auto Affine3DTo2DSwapAxesTransform() -> std::shared_ptr<Affine>;
		static auto CreateBursaWolfParameters(double dX, double dY, double dZ, double Rx, double Ry, double Rz, double dS) -> std::shared_ptr<CoordinateSystems::IProjection>;
		static auto BurwaWolf(double dX, double dY, double dZ, double Rx, double Ry, double Rz, double dS) -> std::shared_ptr<Affine>;

		static auto CreateCoordinateFrameRotationParameters(double dX, double dY, double dZ, double Rx, double Ry, double Rz, double dS) -> std::shared_ptr<CoordinateSystems::IProjection>;
		static auto CoordinateFrameRotation(double dX, double dY, double dZ, double Rx, double Ry, double Rz, double dS) -> std::shared_ptr<Affine>;

		static auto CreateXyzShiftParameters(double dx, double dy, double dz) -> std::shared_ptr<CoordinateSystems::IProjection>;
#pragma endregion
#pragma region IMathTransform members
		auto GetWkt() const -> std::string override;
		auto GetSourceDimension() const -> int override;
		auto GetTargetDimension() const -> int override;
		auto GetIsIdentity() const -> bool override;
		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;
		// In-place matrix multiplication: no heap allocation per point.
		auto TransformPoints(std::span<double const> sourcePoints) const -> std::vector<double> override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
