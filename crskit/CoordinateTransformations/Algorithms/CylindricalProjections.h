#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "Authalic.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Common parameters: semi-axes, eccentricity, central meridian and false origin.
	class SimpleProjection
	: public MapProjectionBase
	{
	protected:
		SimpleProjection(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Value(char const* name) const -> double;

		double _a;
		double _e;
		double _longitudeOfOrigin;
		double _falseEasting;
		double _falseNorthing;
	};

	// Mercator (1SP) (Spherical), 9841: radius R = RA, the authalic radius (a on a sphere).
	class MercatorSpherical final : public SimpleProjection
	{
		double _radius;
		double _k0;

	public:
		MercatorSpherical(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Equal Earth, 1078.
	class EqualEarth final : public SimpleProjection
	{
		Authalic _authalic;
		double _rq;

	public:
		EqualEarth(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Lambert Cylindrical Equal Area, 9835 (ellipsoid) and 9834 (spherical, radius RA).
	class LambertCylindricalEqualArea : public SimpleProjection
	{
	protected:
		LambertCylindricalEqualArea(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse, bool spherical);

	private:
		Authalic _authalic;
		bool _spherical;
		double _k0;		// ellipsoid: scale on the standard parallel; sphere: cos(lat1)
		double _radius;	// sphere: RA

	public:
		LambertCylindricalEqualArea(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	class LambertCylindricalEqualAreaSpherical final : public LambertCylindricalEqualArea
	{
	public:
		LambertCylindricalEqualAreaSpherical(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Equidistant Cylindrical, 1028 and 9842, and its spherical form 1029 and 9823. On a sphere the ellipsoidal formulas
	// are the spherical ones; on an ellipsoid (EPSG 32662, WGS 84 / Plate Carree) PROJ applies the ellipsoidal formulas to
	// the spherical method too, and so does this class.
	class EquidistantCylindrical : public SimpleProjection
	{
		double _n;			// third flattening, for the meridian arc
		double _arcScale;	// a / (1 + n) (1 + n^2 / 4 + n^4 / 64)
		double _parallelRadius;

		auto MeridianArc(double phi) const -> double;

	public:
		EquidistantCylindrical(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	class EquidistantCylindricalSpherical : public EquidistantCylindrical
	{
	public:
		EquidistantCylindricalSpherical(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// The deprecated EPSG codes 9842 and 9823 of the same methods, with other parameter codes.
	class EquidistantCylindricalDeprecated final : public EquidistantCylindrical
	{
	public:
		using EquidistantCylindrical::EquidistantCylindrical;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	class EquidistantCylindricalSphericalDeprecated final : public EquidistantCylindricalSpherical
	{
	public:
		using EquidistantCylindricalSpherical::EquidistantCylindricalSpherical;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Lambert Azimuthal Equal Area (Spherical), 1027: radius RA.
	class LambertAzimuthalEqualAreaSpherical final : public SimpleProjection
	{
		double _radius;
		double _latitudeOfOrigin;

	public:
		LambertAzimuthalEqualAreaSpherical(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};
}