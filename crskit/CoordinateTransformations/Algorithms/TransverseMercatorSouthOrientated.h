#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"
#include "TransverseMercator.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Westing W = FE - E' and southing S = FN - N', with (E', N') the Transverse Mercator coordinates of the same
	// point for a zero false origin (GN7-2, method 9808).
	class TransverseMercatorSouthOrientated final
	: public MapProjectionBase
	{
		double _falseEasting;
		double _falseNorthing;
		std::shared_ptr<TransverseMercator> _transverseMercator;

	public:
		TransverseMercatorSouthOrientated(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

#pragma region MapProjectionBase members
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
#pragma endregion
#pragma region IMathTransform members
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}