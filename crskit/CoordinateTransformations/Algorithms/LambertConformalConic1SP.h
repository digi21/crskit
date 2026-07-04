#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	class LambertConformalConic1SP final
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
		double _m0;
		double _t0;
		double _n;
		double _f;
		double _r0;
		double _e;
#pragma endregion

#pragma region Constructors

	public:
		LambertConformalConic1SP(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
#pragma endregion
#pragma region MapProjectionBase members
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
#pragma endregion
#pragma region IMathTransform members

	public:
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
