#pragma once

#include <array>
#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Krovak (9819) and Krovak Modified (1042) of GN7-2. The formulas yield (southing, westing); the North Orientated
	// methods (1041, 1043) are the same formulas on a CRS with axes east and north, and CreateProjectionTransforms maps one
	// onto the other.
	class Krovak
	: public MapProjectionBase
	{
	protected:
		Krovak(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse, bool modified);

	private:
		double _e;
		double _longitudeOfOrigin;
		double _alphaC;
		double _latitudeP;
		double _falseEasting;
		double _falseNorthing;
		double _b;
		double _t0;
		double _n;
		double _r0;
		bool _modified;
		double _x0{};
		double _y0{};
		std::array<double, 10> _c{};

		// Correction (dX, dY) of the modified method at reduced coordinates (xr, yr).
		auto Correction(double xr, double yr, double& dx, double& dy) const -> void;

	public:
		Krovak(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	class KrovakNorthOrientated final : public Krovak
	{
	public:
		using Krovak::Krovak;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	class KrovakModified : public Krovak
	{
	public:
		KrovakModified(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};

	class KrovakModifiedNorthOrientated final : public KrovakModified
	{
	public:
		using KrovakModified::KrovakModified;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};
}