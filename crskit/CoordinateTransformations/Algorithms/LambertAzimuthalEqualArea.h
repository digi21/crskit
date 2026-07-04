#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	class LambertAzimuthalEqualArea final
	: public MapProjectionBase
	{

#pragma region Parameters
		double _semiMinorAxis;
		double _semiMajorAxis;
		double _projectionCenterLatitude;
		double _projectionCenterLongitude;
		double _falseEasting;
		double _falseNorthing;
#pragma endregion
#pragma region Constants to speed up the computation
		double _e;

		double _latitude0;
		double _longitude0;
		double _sinlat0;
		double _q0;
		double _qp;
		double _beta0;
		double _rq;
		double _d;
#pragma endregion

#pragma region Constructor

	public:
		LambertAzimuthalEqualArea(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
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
