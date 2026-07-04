#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	class ObliqueMercator final
	: public MapProjectionBase
	{

#pragma region Parameters
		double _semiMinorAxis;
		double _semiMajorAxis;
		double _projectionCenterLatitude;
		double _projectionCenterLongitude;
		double _initialLineAzimuth;
		double _rectifiedGridAngle;
		double _scaleAtCenter;
		double _eastingAtProjectionCenter;
		double _northingAtProjectionCenter;
#pragma endregion
#pragma region Constants to speed up the computation
		double _e;
		double _b;
		double _a;
		double _t0;
		double _d;
		double _f;
		double _h;
		double _g;
		double _gamma0;
		double _longitude0;
		double _vc;
		double _uc;
#pragma endregion

#pragma region Constructor

	public:
		ObliqueMercator(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
#pragma endregion
#pragma region MapProjectionBase members
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
#pragma endregion
#pragma region IMathTransform members
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
