#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "CylindricalProjections.h"
#include "TransverseMercator.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Meridian arc of GN7-2 (series to e^6) and its reverse (footpoint latitude series).
	class MeridianArcSeries
	{
		double _a;
		double _e;

	public:
		MeridianArcSeries(double a, double e) : _a{a}, _e{e} {}
		[[nodiscard]] auto M(double phi) const -> double;
		[[nodiscard]] auto Footpoint(double m) const -> double;
	};

	// Bonne (South Orientated), 9828: yields (westing, southing).
	class BonneSouthOrientated final : public SimpleProjection
	{
		MeridianArcSeries _arc;
		double _latitudeOfOrigin;
		double _apex;	// a mO / sin(latO)
		double _mO;

	public:
		BonneSouthOrientated(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Hyperbolic Cassini-Soldner, 9833 (Vanua Levu): Cassini-Soldner with N = FN + X - X^3 / (6 rho nu).
	class HyperbolicCassiniSoldner final : public SimpleProjection
	{
		MeridianArcSeries _arc;
		double _latitudeOfOrigin;
		double _mO;

	public:
		HyperbolicCassiniSoldner(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Lambert Conic Near-Conformal, 9817 (Levant): the meridian distance truncated to a cubic.
	class LambertConicNearConformal final : public SimpleProjection
	{
		double _latitudeOfOrigin;
		double _k0;
		double _bigA;
		double _coefficients[5];	// A' (per degree), B', C', D', E'
		double _r0;
		double _s0;

		auto S(double phi) const -> double;

	public:
		LambertConicNearConformal(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Transverse Mercator Zoned Grid System, 9824 (UTM grid systems): the zone comes from the longitude and its number
	// times 10^6 is added to the easting.
	class TransverseMercatorZonedGrid final : public MapProjectionBase
	{
		double _initialLongitude;	// degrees
		double _zoneWidth;			// degrees
		double _falseEasting;
		std::shared_ptr<TransverseMercator> _transverseMercator;	// central meridian 0, false easting 0

		auto Zone(double longitudeDegrees) const -> int;

	public:
		TransverseMercatorZonedGrid(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// New Zealand Map Grid, 9811: complex polynomial series of GN7-2.
	class NewZealandMapGrid final : public SimpleProjection
	{
		double _latitudeOfOrigin;

	public:
		NewZealandMapGrid(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Tunisia Mining Grid, 9816: linear in grads and kilometres, with a different northing scale on each side of the origin.
	class TunisiaMiningGrid final : public SimpleProjection
	{
		double _latitudeOfOrigin;	// grads

	public:
		TunisiaMiningGrid(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};
}