#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "CylindricalProjections.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	namespace
	{
		auto Eccentricity(double a, double b) -> double
		{
			auto const f = (a - b) / a;
			return sqrt(2 * f - f * f);
		}

		// Equal Earth polynomial of theta and its derivative (GN7-2).
		auto EqualEarthN(double theta) -> double
		{
			auto const t2 = theta * theta;
			auto const t6 = t2 * t2 * t2;
			return theta * (1.340264 - 0.081106 * t2 + t6 * (0.000893 + 0.003796 * t2));
		}

		auto EqualEarthDerivative(double theta) -> double
		{
			auto const t2 = theta * theta;
			auto const t6 = t2 * t2 * t2;
			return 1.340264 - 0.243318 * t2 + t6 * (0.006251 + 0.034164 * t2);
		}
	}

	SimpleProjection::SimpleProjection(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _a{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _e{Eccentricity(_a, std::get<double>(parameters->GetParameter("semi_minor").GetValue()))}
		, _longitudeOfOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter(
			std::ranges::any_of(parameters->GetParameters(), [](auto const& p) { return 0 == compareNoCase(p.GetName().c_str(), "longitude_of_center"); })
				? "longitude_of_center" : "central_meridian").GetValue()))}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
	}

	auto SimpleProjection::Value(char const* name) const -> double
	{
		return std::get<double>(_parameters->GetParameter(name).GetValue());
	}

#pragma region Mercator (1SP) (Spherical)
	MercatorSpherical::MercatorSpherical(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _radius{Authalic{_e}.Radius(_a)}
		, _k0{Value("scale_factor")}
	{
	}

	auto MercatorSpherical::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		x = _falseEasting + _radius * _k0 * LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		y = _falseNorthing + _radius * _k0 * log(tan(M_PI / 4 + phi / 2));
	}

	auto MercatorSpherical::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		phi = M_PI / 2 - 2 * atan(exp(-(y - _falseNorthing) / (_radius * _k0)));
		lambda = _longitudeOfOrigin + (x - _falseEasting) / (_radius * _k0);
	}

	auto MercatorSpherical::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<MercatorSpherical>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Equal Earth
	EqualEarth::EqualEarth(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _authalic{_e}
		, _rq{_authalic.Radius(_a)}
	{
	}

	auto EqualEarth::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const theta = asin(sin(_authalic.Beta(phi)) * sqrt(3.0) / 2);
		x = _falseEasting + _rq * 2 * LongitudeFromOrigin(lambda, _longitudeOfOrigin) * cos(theta) / (sqrt(3.0) * EqualEarthDerivative(theta));
		y = _falseNorthing + _rq * EqualEarthN(theta);
	}

	auto EqualEarth::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const target = (y - _falseNorthing) / _rq;
		auto theta = target;
		for (auto i = 0; i < 20; ++i)
		{
			auto const delta = (EqualEarthN(theta) - target) / EqualEarthDerivative(theta);
			theta -= delta;
			if (fabs(delta) < 1e-15)
				break;
		}
		auto const ratio = 2 * sin(theta) / sqrt(3.0);
		phi = _authalic.Phi(asin(ratio < -1 ? -1 : ratio > 1 ? 1 : ratio));
		lambda = _longitudeOfOrigin + sqrt(3.0) * (x - _falseEasting) * EqualEarthDerivative(theta) / (2 * _rq * cos(theta));
	}

	auto EqualEarth::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<EqualEarth>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Lambert Cylindrical Equal Area
	LambertCylindricalEqualArea::LambertCylindricalEqualArea(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: LambertCylindricalEqualArea{parameters, inverse, false}
	{
	}

	LambertCylindricalEqualArea::LambertCylindricalEqualArea(std::shared_ptr<IProjection> const& parameters, bool inverse, bool spherical)
		: SimpleProjection{parameters, inverse}
		, _authalic{_e}
		, _spherical{spherical}
	{
		auto const lat1 = DEGREES_TO_RADIANS(Value("standard_parallel1"));
		_radius = _authalic.Radius(_a);
		_k0 = spherical ? cos(lat1) : cos(lat1) / sqrt(1 - _e * _e * sin(lat1) * sin(lat1));
	}

	auto LambertCylindricalEqualArea::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const dLambda = LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		if (_spherical)
		{
			x = _falseEasting + _radius * dLambda * _k0;
			y = _falseNorthing + _radius * sin(phi) / _k0;
			return;
		}
		x = _falseEasting + _a * _k0 * dLambda;
		y = _falseNorthing + _a * _authalic.Q(phi) / (2 * _k0);
	}

	auto LambertCylindricalEqualArea::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const dE = x - _falseEasting;
		auto const dN = y - _falseNorthing;
		if (_spherical)
		{
			auto const ratio = dN * _k0 / _radius;
			phi = asin(ratio < -1 ? -1 : ratio > 1 ? 1 : ratio);
			lambda = _longitudeOfOrigin + dE / (_radius * _k0);
			return;
		}
		auto const ratio = 2 * dN * _k0 / (_a * _authalic.QP());
		phi = _authalic.Phi(asin(ratio < -1 ? -1 : ratio > 1 ? 1 : ratio));
		lambda = _longitudeOfOrigin + dE / (_a * _k0);
	}

	auto LambertCylindricalEqualArea::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertCylindricalEqualArea>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	LambertCylindricalEqualAreaSpherical::LambertCylindricalEqualAreaSpherical(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: LambertCylindricalEqualArea{parameters, inverse, true}
	{
	}

	auto LambertCylindricalEqualAreaSpherical::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertCylindricalEqualAreaSpherical>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Equidistant Cylindrical
	EquidistantCylindrical::EquidistantCylindrical(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
	{
		auto const lat1 = DEGREES_TO_RADIANS(Value("standard_parallel1"));
		auto const b = _a * sqrt(1 - _e * _e);
		_n = (_a - b) / (_a + b);
		auto const n2 = _n * _n;
		_arcScale = _a / (1 + _n) * (1 + n2 / 4 + n2 * n2 / 64);
		_parallelRadius = _a * cos(lat1) / sqrt(1 - _e * _e * sin(lat1) * sin(lat1));
	}

	// Meridian arc from the equator (Helmert's series in the third flattening, to n^4: about 1e-5 m).
	auto EquidistantCylindrical::MeridianArc(double phi) const -> double
	{
		auto const n = _n;
		auto const n2 = n * n;
		auto const n3 = n2 * n;
		auto const n4 = n2 * n2;
		return _arcScale * (phi
			- (3 * n / 2 - 9 * n3 / 16) * sin(2 * phi)
			+ (15 * n2 / 16 - 15 * n4 / 32) * sin(4 * phi)
			- (35 * n3 / 48) * sin(6 * phi)
			+ (315 * n4 / 512) * sin(8 * phi));
	}

	auto EquidistantCylindrical::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		x = _falseEasting + _parallelRadius * LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		y = _falseNorthing + MeridianArc(phi);
	}

	auto EquidistantCylindrical::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		lambda = _longitudeOfOrigin + (x - _falseEasting) / _parallelRadius;
		auto const arc = y - _falseNorthing;
		phi = arc / _arcScale;
		auto const e2 = _e * _e;
		for (auto i = 0; i < 10; ++i)
		{
			auto const s = sin(phi);
			auto const derivative = _a * (1 - e2) / pow(1 - e2 * s * s, 1.5);
			auto const delta = (MeridianArc(phi) - arc) / derivative;
			phi -= delta;
			if (fabs(delta) < 1e-15)
				break;
		}
	}

	auto EquidistantCylindrical::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<EquidistantCylindrical>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	EquidistantCylindricalSpherical::EquidistantCylindricalSpherical(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: EquidistantCylindrical{parameters, inverse}
	{
	}

	auto EquidistantCylindricalSpherical::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<EquidistantCylindricalSpherical>(_parameters, !_inverse);
		return _inverseTransformation;
	}
	auto EquidistantCylindricalDeprecated::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<EquidistantCylindricalDeprecated>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	auto EquidistantCylindricalSphericalDeprecated::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<EquidistantCylindricalSphericalDeprecated>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Lambert Azimuthal Equal Area (Spherical)
	LambertAzimuthalEqualAreaSpherical::LambertAzimuthalEqualAreaSpherical(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _radius{Authalic{_e}.Radius(_a)}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_center"))}
	{
	}

	auto LambertAzimuthalEqualAreaSpherical::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const dLambda = LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		auto const denominator = 1 + sin(_latitudeOfOrigin) * sin(phi) + cos(_latitudeOfOrigin) * cos(phi) * cos(dLambda);
		auto const k = denominator > 0 ? sqrt(2 / denominator) : 0.0;
		x = _falseEasting + _radius * k * cos(phi) * sin(dLambda);
		y = _falseNorthing + _radius * k * (cos(_latitudeOfOrigin) * sin(phi) - sin(_latitudeOfOrigin) * cos(phi) * cos(dLambda));
	}

	auto LambertAzimuthalEqualAreaSpherical::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const dE = x - _falseEasting;
		auto const dN = y - _falseNorthing;
		auto const rho = hypot(dE, dN);
		if (rho == 0.0)
		{
			phi = _latitudeOfOrigin;
			lambda = _longitudeOfOrigin;
			return;
		}
		auto const ratio = rho / (2 * _radius);
		auto const c = 2 * asin(ratio > 1 ? 1 : ratio);
		auto const sinPhi = cos(c) * sin(_latitudeOfOrigin) + dN * sin(c) * cos(_latitudeOfOrigin) / rho;
		phi = asin(sinPhi < -1 ? -1 : sinPhi > 1 ? 1 : sinPhi);
		lambda = _longitudeOfOrigin + atan2(dE * sin(c), rho * cos(_latitudeOfOrigin) * cos(c) - dN * sin(_latitudeOfOrigin) * sin(c));
	}

	auto LambertAzimuthalEqualAreaSpherical::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertAzimuthalEqualAreaSpherical>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion
}