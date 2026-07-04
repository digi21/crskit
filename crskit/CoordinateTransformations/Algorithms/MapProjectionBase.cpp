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
