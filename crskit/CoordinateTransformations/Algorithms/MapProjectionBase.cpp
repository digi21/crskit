#include "pch.h"
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	MapProjectionBase::MapProjectionBase(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MathTransformBase{parameters, inverse}
	{
	}

	auto MapProjectionBase::RefineReverse(double x, double y, double& lambda, double& phi) const -> void
	{
		constexpr double h = 1e-7;  // radians, about 0.6 m: the step of the numerical Jacobian
		for (auto i = 0; i < 8; ++i)
		{
			double x0, y0;
			Geo2Proj(lambda, phi, x0, y0);
			auto const dx = x - x0;
			auto const dy = y - y0;
			if (!std::isfinite(dx) || !std::isfinite(dy) || std::hypot(dx, dy) < 1e-9)
				return;

			double xl, yl, xp, yp;
			Geo2Proj(lambda + h, phi, xl, yl);
			Geo2Proj(lambda, phi + h, xp, yp);
			auto const a = (xl - x0) / h;
			auto const b = (xp - x0) / h;
			auto const c = (yl - y0) / h;
			auto const d = (yp - y0) / h;
			auto const det = a * d - b * c;
			if (!std::isfinite(det) || det == 0.0)
				return;

			lambda += (d * dx - b * dy) / det;
			phi += (a * dy - c * dx) / det;
		}
	}

	auto MapProjectionBase::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		vector<double> result;
		result.resize(point.size());

		if (!_inverse)
		{
			auto lambda = DEGREES_TO_RADIANS(point[0]);
			auto const phi = DEGREES_TO_RADIANS(point[1]);

			// Normalise longitude to [-pi, pi] in O(1). A while-loop here hangs when lambda is huge or
			// non-finite (subtracting 2*pi from a value >= ~2^53*pi never changes it). Leave non-finite
			// values untouched (a true out-of-domain result stays inf rather than becoming NaN).
			if (std::isfinite(lambda))
				lambda = std::remainder(lambda, 2 * M_PI);

			double x, y;
			Geo2Proj(lambda, phi, x, y);
			result[0] = x;
			result[1] = y;
		}
		else
		{
			double lambda, phi;
			Proj2Geo(point[0], point[1], lambda, phi);

			// Normalise longitude to [-pi, pi] in O(1). A while-loop here hangs when lambda is huge or
			// non-finite (subtracting 2*pi from a value >= ~2^53*pi never changes it). Leave non-finite
			// values untouched (a true out-of-domain result stays inf rather than becoming NaN).
			if (std::isfinite(lambda))
				lambda = std::remainder(lambda, 2 * M_PI);

			result[0] = RADIANS_TO_DEGREES(lambda);
			result[1] = RADIANS_TO_DEGREES(phi);
		}

		return result;
	}
}
