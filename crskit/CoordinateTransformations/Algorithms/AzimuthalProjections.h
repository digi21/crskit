#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "CylindricalProjections.h"
#include "Geodesic.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Colombia Urban, 1052: a projection plane at height hO above the ellipsoid.
	class ColombiaUrban final : public SimpleProjection
	{
		double _latitudeOfOrigin;
		double _rhoO;
		double _nuO;
		double _bigA;
		double _bigB;
		double _height;

	public:
		ColombiaUrban(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Azimuthal Equidistant, 1125: geodesic distance and azimuth from the origin.
	class AzimuthalEquidistant final : public SimpleProjection
	{
		double _latitudeOfOrigin;
		Geodesic _geodesic;

	public:
		AzimuthalEquidistant(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Modified Azimuthal Equidistant, 9832 (Yap Islands): GN7-2 series for short distances.
	class ModifiedAzimuthalEquidistant final : public SimpleProjection
	{
		double _latitudeOfOrigin;
		double _nuO;

	public:
		ModifiedAzimuthalEquidistant(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Guam Projection, 9831.
	class GuamProjection final : public SimpleProjection
	{
		double _latitudeOfOrigin;
		double _mO;

		auto MeridianArc(double phi) const -> double;

	public:
		GuamProjection(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Local Orthographic, 1130: the ellipsoidal Orthographic (9840) about the projection centre, rotated by the azimuth
	// and scaled.
	class LocalOrthographic final : public SimpleProjection
	{
		double _latitudeOfCentre;
		double _nuC;
		double _azimuth;
		double _scale;

	public:
		LocalOrthographic(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};
}