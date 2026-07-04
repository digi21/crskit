#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	class Mercator1Sp final
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
		double _latitude1;
		double _longitude0;
		double _e;
		double _e2;
		double _e4;
		double _e6;
		double _e8;
		double _k0;
#pragma endregion

#pragma region Constructor

	public:
		Mercator1Sp(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
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
