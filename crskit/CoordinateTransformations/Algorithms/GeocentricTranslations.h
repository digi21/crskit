#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "Afine.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class GeocentricTranslations
	: public Affine
	{

	public:
		GeocentricTranslations(double dx, double dy, double dz);
		explicit GeocentricTranslations(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		GeocentricTranslations(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
	};
}
