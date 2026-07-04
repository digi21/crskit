#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "Afine.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class PositionVectorTransformation
	: public Affine
	{

	public:
		explicit PositionVectorTransformation(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		PositionVectorTransformation(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		PositionVectorTransformation(double dX, double dY, double dZ, double Rx, double Ry, double Rz, double dS);
	};
}
