#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Polar Stereographic, variants A (9810), B (9829) and C (9830) of GN7-2. All three are rho = K t and
	// N = FN' -+ rho cos(lon - lonO); the variant only sets K and FN'.
	class PolarStereographic
	: public MapProjectionBase
	{
	protected:
		enum class Variant { AOrB, C };

		PolarStereographic(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse, Variant variant);

	private:
		bool _south;
		double _e;
		double _longitudeOfOrigin;
		double _falseEasting;
		double _falseNorthing;	// FN', the northing of the pole
		double _k;				// rho = _k t

		auto T(double phi) const -> double;

	public:
		// Variant A, or variant B when there is no scale factor or the latitude of origin is not a pole (the WKT1 form GDAL
		// writes for variant B: PROJECTION["Polar_Stereographic"] with latitude_of_origin as the standard parallel).
		PolarStereographic(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Registered for EPSG 9829 with the variant B parameters; same WKT1 name and computation as PolarStereographic.
	class PolarStereographicVariantB final
	: public PolarStereographic
	{
	public:
		PolarStereographicVariantB(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	// Variant C: the false easting and northing are those of the point where the standard parallel crosses lonO.
	class PolarStereographicVariantC final
	: public PolarStereographic
	{
	public:
		PolarStereographicVariantC(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};
}