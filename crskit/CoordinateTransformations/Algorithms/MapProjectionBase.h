#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../MathTransformBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Clamps to [-1, 1] so asin/acos do not return NaN when floating-point rounding pushes an argument
	// that is mathematically within range a hair past the limit (e.g. sin(b)*cosh(eta) at the domain edge).
	[[nodiscard]] inline auto clampToUnit(double value) -> double
	{
		return value < -1.0 ? -1.0 : (value > 1.0 ? 1.0 : value);
	}

	class MapProjectionBase : public MathTransformBase
	{
	public:
		MapProjectionBase(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

	public:
		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;

		virtual auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void = 0;
		virtual auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void = 0;
	};
}
