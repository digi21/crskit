#pragma once

#include <algorithm>
#include <iterator>
#include <vector>

#include "../Mathematics/Angles.h"

namespace CrsKit::CoordinateTransformations
{
	class UnitsOfMeasure final
	{
	public:
		static auto DegreesToGrads(std::vector<double> const& unitsInDegrees) -> std::vector<double>
		{
			std::vector<double> unitsInGrads;
			unitsInGrads.reserve(unitsInDegrees.size());

			std::ranges::transform(unitsInDegrees, std::back_inserter(unitsInGrads),
				[](auto coordinate) { return CrsKit::Math::DEGREES_TO_GRADS(coordinate); });

			return unitsInGrads;
		}
	};
}
