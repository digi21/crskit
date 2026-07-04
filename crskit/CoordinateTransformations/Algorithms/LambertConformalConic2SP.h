#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	class LambertConformalConic2SP final
	: public MapProjectionBase
	{

#pragma region Parameters
		double _semiMinorAxis;
		double _semiMajorAxis;
		double _falseOriginLatitude;
		double _falseOriginLongitude;
		double _firstStandardParallelLatitude;
		double _secondStandardParallelLatitude;
		double _falseEasting;
		double _falseNorthing;
#pragma endregion
#pragma region Constants to speed up the computation
		double _e;
		double _m1;
		double _m2;
		double _t1;
		double _t2;
		double _tf;
		double _n;
		double _f;
		double _rf;
#pragma endregion

#pragma region Constructor

	public:
		LambertConformalConic2SP(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
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
