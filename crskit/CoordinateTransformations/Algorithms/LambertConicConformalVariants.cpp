#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "LambertConicConformalVariants.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	LambertConicConformalVariant::LambertConicConformalVariant(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
	{
		_a = Value("semi_major");
		auto const f = (_a - Value("semi_minor")) / _a;
		_e = sqrt(2 * f - f * f);
	}

	auto LambertConicConformalVariant::Value(char const* name) const -> double
	{
		return std::get<double>(_parameters->GetParameter(name).GetValue());
	}

	auto LambertConicConformalVariant::T(double phi) const -> double
	{
		return tan(M_PI / 4 - phi / 2) / pow((1 - _e * sin(phi)) / (1 + _e * sin(phi)), _e / 2);
	}

	auto LambertConicConformalVariant::M(double phi) const -> double
	{
		return cos(phi) / sqrt(1 - _e * _e * sin(phi) * sin(phi));
	}

	auto LambertConicConformalVariant::OneStandardParallel(double latO, double k0) -> void
	{
		_c.n = sin(latO);
		_c.f = M(latO) / (_c.n * pow(T(latO), _c.n));
		_c.k = k0;
	}

	auto LambertConicConformalVariant::TwoStandardParallels(double lat1, double lat2) -> void
	{
		_c.n = fabs(lat1 - lat2) < 1e-12 ? sin(lat1) : (log(M(lat1)) - log(M(lat2))) / (log(T(lat1)) - log(T(lat2)));
		_c.f = M(lat1) / (_c.n * pow(T(lat1), _c.n));
	}

	auto LambertConicConformalVariant::Radius(double phi) const -> double
	{
		return _a * _c.k * _c.f * pow(T(phi), _c.n);
	}

	auto LambertConicConformalVariant::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const r = Radius(phi);
		auto const theta = _c.n * LongitudeFromOrigin(lambda, _c.longitudeOfOrigin) - _c.alpha;
		x = _c.falseEasting + (_c.west ? -r : r) * sin(theta);
		y = _c.falseNorthing + _c.rF - r * cos(theta);
	}

	auto LambertConicConformalVariant::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const dE = _c.west ? _c.falseEasting - x : x - _c.falseEasting;
		auto const dN = _c.rF - (y - _c.falseNorthing);
		auto const sign = _c.n < 0 ? -1.0 : 1.0;
		auto const r = sign * hypot(dE, dN);
		auto const t = pow(r / (_a * _c.k * _c.f), 1 / _c.n);
		auto const theta = atan2(sign * dE, sign * dN);
		lambda = (theta + _c.alpha) / _c.n + _c.longitudeOfOrigin;

		phi = M_PI / 2 - 2 * atan(t);
		for (auto i = 0; i < 20; ++i)
		{
			auto const next = M_PI / 2 - 2 * atan(t * pow((1 - _e * sin(phi)) / (1 + _e * sin(phi)), _e / 2));
			auto const delta = fabs(next - phi);
			phi = next;
			if (delta < 1e-14)
				break;
		}
	}

	LambertConicConformalWestOrientated::LambertConicConformalWestOrientated(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: LambertConicConformalVariant{parameters, inverse}
	{
		auto const latO = DEGREES_TO_RADIANS(Value("latitude_of_origin"));
		OneStandardParallel(latO, Value("scale_factor"));
		_c.rF = Radius(latO);
		_c.west = true;
		_c.longitudeOfOrigin = DEGREES_TO_RADIANS(Value("central_meridian"));
		_c.falseEasting = Value("false_easting");
		_c.falseNorthing = Value("false_northing");
	}

	auto LambertConicConformalWestOrientated::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertConicConformalWestOrientated>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	LambertConicConformal1SPVariantB::LambertConicConformal1SPVariantB(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: LambertConicConformalVariant{parameters, inverse}
	{
		OneStandardParallel(DEGREES_TO_RADIANS(Value("latitude_of_origin")), Value("scale_factor"));
		_c.rF = Radius(DEGREES_TO_RADIANS(Value("latitude_of_false_origin")));
		_c.longitudeOfOrigin = DEGREES_TO_RADIANS(Value("central_meridian"));
		_c.falseEasting = Value("false_easting");
		_c.falseNorthing = Value("false_northing");
	}

	auto LambertConicConformal1SPVariantB::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertConicConformal1SPVariantB>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	LambertConicConformal2SPMichigan::LambertConicConformal2SPMichigan(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: LambertConicConformalVariant{parameters, inverse}
	{
		TwoStandardParallels(DEGREES_TO_RADIANS(Value("standard_parallel1")), DEGREES_TO_RADIANS(Value("standard_parallel2")));
		_c.k = Value("ellipsoid_scaling_factor");
		_c.rF = Radius(DEGREES_TO_RADIANS(Value("latitude_of_origin")));
		_c.longitudeOfOrigin = DEGREES_TO_RADIANS(Value("central_meridian"));
		_c.falseEasting = Value("false_easting");
		_c.falseNorthing = Value("false_northing");
	}

	auto LambertConicConformal2SPMichigan::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertConicConformal2SPMichigan>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	LambertConicConformal2SPBelgium::LambertConicConformal2SPBelgium(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: LambertConicConformalVariant{parameters, inverse}
	{
		TwoStandardParallels(DEGREES_TO_RADIANS(Value("standard_parallel1")), DEGREES_TO_RADIANS(Value("standard_parallel2")));
		_c.rF = Radius(DEGREES_TO_RADIANS(Value("latitude_of_origin")));
		_c.alpha = DEGREES_TO_RADIANS(29.2985 / 3600.0);
		_c.longitudeOfOrigin = DEGREES_TO_RADIANS(Value("central_meridian"));
		_c.falseEasting = Value("false_easting");
		_c.falseNorthing = Value("false_northing");
	}

	auto LambertConicConformal2SPBelgium::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertConicConformal2SPBelgium>(_parameters, !_inverse);
		return _inverseTransformation;
	}
}