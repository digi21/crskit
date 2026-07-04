#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	class AlbersConicEqualArea final : public MapProjectionBase
	{

#pragma region Parameters

	private:
		double _semiMinorAxis;
		double _semiMajorAxis;
		double _firstStandardParallelLatitude;
		double _secondStandardParallelLatitude;
		double _falseOriginLatitude;
		double _falseOriginLongitude;
		double _falseEasting;
		double _falseNorthing;
#pragma endregion
#pragma region Constants to speed up the computation
		double _e;
		double _e2;
		double _latitude0;
		double _longitude0;
		double _latitude1;
		double _latitude2;

		double _sinlat0;
		double _sinlat1;
		double _sinlat2;
		double _coslat1;
		double _coslat2;

		double _alfa0;
		double _alfa1;
		double _alfa2;
		double _m1;
		double _m2;
		double _n;
		double _c;

		double _p0;
#pragma endregion

#pragma region Constructor

	public:
		AlbersConicEqualArea(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
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
