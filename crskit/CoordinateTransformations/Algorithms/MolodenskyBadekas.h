#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "Afine.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	// Transformation Molodensky-Badekas de 10 parameters (convencion Coordinate
	// Frame rotation), EPSG Coordinate Operation Method 9636. Operates on
	// geocentric coordinates (X, Y, Z). It is an affine transformation: with respect to
	// a 7-parameter frame rotation adds an evaluation point P, so
	// that Xt = M*(Xs - P) + P + t = M*Xs + (t + (I - M)*P).
	class MolodenskyBadekas
	: public Affine
	{

	public:
		MolodenskyBadekas(double dx, double dy, double dz, double rx, double ry, double rz, double ds, double px, double py, double pz);
		explicit MolodenskyBadekas(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		MolodenskyBadekas(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
	};
}
