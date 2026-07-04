#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	class ObliqueStereographic final
	: public MapProjectionBase
	{

#pragma region Parameters
		double _semiMinorAxis;
		double _semiMajorAxis;
		double _latitudeOfNaturalOrigin;
		double _longitudeOfNaturalOrigin;
		double _scaleFactorAtNaturalOrigin;
		double _falseEasting;
		double _falseNorthing;
#pragma endregion
#pragma region Constants to speed up the computation
		double _a;
		double _b;
		double _f;
		double _e;
		double _p0;
		double _v0;
		double _r;
		double _n;
		double _s1;
		double _s2;
		double _w1;
		double _c;
		double _w2;
		double _x0;
		double _l0;
		double _g;
		double _h;
#pragma endregion

#pragma region Constructor

	public:
		ObliqueStereographic(double semiMajorAxis, double semiMinorAxis, double latitudeOfOrigin, double centralMeridian, double scaleFactor, double falseEasting, double falseNorthing, bool inverse);
		ObliqueStereographic(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
#pragma endregion
#pragma region MapProjectionBase members
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
#pragma endregion
#pragma region IMathTransform members
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
#pragma region Implementation

	private:
		auto ComputeConstants() -> void;
#pragma endregion
	};
}
