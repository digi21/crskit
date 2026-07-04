#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "Afine.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class CoordinateFrameRotation
	: public Affine
	{

	public:
		CoordinateFrameRotation(double dx, double dy, double dz, double rX, double rY, double rZ, double dS);
		explicit CoordinateFrameRotation(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		CoordinateFrameRotation(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
	};
}
