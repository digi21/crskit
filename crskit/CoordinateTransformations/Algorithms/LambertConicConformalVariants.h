#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Lambert Conic Conformal variants of GN7-2 that share one computation:
	//   E = EF +- r sin(theta - alpha), N = NF + rF - r cos(theta - alpha), r = a k F t^n, theta = n (lon - lonO).
	// Each variant sets n, F, k, rF, alpha and the sign of E from its parameters.
	class LambertConicConformalVariant
	: public MapProjectionBase
	{
	protected:
		struct Constants
		{
			double n{};
			double f{};
			double k{ 1.0 };
			double rF{};
			double alpha{};
			bool west{};
			double longitudeOfOrigin{};
			double falseEasting{};
			double falseNorthing{};
		};

		LambertConicConformalVariant(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

		auto Value(char const* name) const -> double;
		auto T(double phi) const -> double;
		auto M(double phi) const -> double;
		// n, F and k for one standard parallel (latitude of natural origin latO, scale factor k0).
		auto OneStandardParallel(double latO, double k0) -> void;
		// n and F for two standard parallels.
		auto TwoStandardParallels(double lat1, double lat2) -> void;
		auto Radius(double phi) const -> double;

		double _a;
		double _e;
		Constants _c;

	public:
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
	};

	// 9826: as LCC 1SP but with axes west and north, W = FE - r sin(theta).
	class LambertConicConformalWestOrientated final : public LambertConicConformalVariant
	{
	public:
		LambertConicConformalWestOrientated(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// 1102: LCC 1SP whose false easting and northing are given at a false origin instead of the natural origin.
	class LambertConicConformal1SPVariantB final : public LambertConicConformalVariant
	{
	public:
		LambertConicConformal1SPVariantB(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// 1051: LCC 2SP with the ellipsoid scaled by K.
	class LambertConicConformal2SPMichigan final : public LambertConicConformalVariant
	{
	public:
		LambertConicConformal2SPMichigan(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// 9803: LCC 2SP with the grid rotated by alpha = 29.2985 arc-seconds.
	class LambertConicConformal2SPBelgium final : public LambertConicConformalVariant
	{
	public:
		LambertConicConformal2SPBelgium(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};
}