#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "AzimuthalProjections.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	namespace
	{
		auto Nu(double a, double e, double phi) -> double
		{
			return a / sqrt(1 - e * e * sin(phi) * sin(phi));
		}

		auto Rho(double a, double e, double phi) -> double
		{
			return a * (1 - e * e) / pow(1 - e * e * sin(phi) * sin(phi), 1.5);
		}

		auto Flattening(double e) -> double
		{
			return 1 - sqrt(1 - e * e);
		}
	}

#pragma region Colombia Urban
	ColombiaUrban::ColombiaUrban(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_origin"))}
		, _rhoO{Rho(_a, _e, _latitudeOfOrigin)}
		, _nuO{Nu(_a, _e, _latitudeOfOrigin)}
		, _height{Value("projection_plane_origin_height")}
	{
		_bigA = 1 + _height / _nuO;
		_bigB = tan(_latitudeOfOrigin) / (2 * _rhoO * _nuO);
	}

	auto ColombiaUrban::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const dLambda = LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		auto const nu = Nu(_a, _e, phi);
		auto const g = 1 + _height / Rho(_a, _e, (_latitudeOfOrigin + phi) / 2);
		x = _falseEasting + _bigA * nu * cos(phi) * dLambda;
		y = _falseNorthing + g * _rhoO * ((phi - _latitudeOfOrigin) + _bigB * dLambda * dLambda * nu * nu * cos(phi) * cos(phi));
	}

	auto ColombiaUrban::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const c = 1 + _height / _a;
		auto const d = _rhoO * (1 + _height / (_a * (1 - _e * _e)));
		auto const dE = (x - _falseEasting) / c;
		phi = _latitudeOfOrigin + (y - _falseNorthing) / d - _bigB * dE * dE;
		lambda = _longitudeOfOrigin + dE / (Nu(_a, _e, phi) * cos(phi));
		// The GN7-2 reverse formulas are approximate (0.1 mm in the Bogota example); Newton makes them the exact inverse.
		RefineReverse(x, y, lambda, phi);
	}

	auto ColombiaUrban::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<ColombiaUrban>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Azimuthal Equidistant
	AzimuthalEquidistant::AzimuthalEquidistant(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_center"))}
		, _geodesic{_a, Flattening(_e)}
	{
	}

	auto AzimuthalEquidistant::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		double s, azimuth;
		_geodesic.Inverse(_latitudeOfOrigin, _longitudeOfOrigin, phi, lambda, s, azimuth);
		x = _falseEasting + s * sin(azimuth);
		y = _falseNorthing + s * cos(azimuth);
	}

	auto AzimuthalEquidistant::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const dE = x - _falseEasting;
		auto const dN = y - _falseNorthing;
		_geodesic.Direct(_latitudeOfOrigin, _longitudeOfOrigin, atan2(dE, dN), hypot(dE, dN), phi, lambda);
	}

	auto AzimuthalEquidistant::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<AzimuthalEquidistant>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Modified Azimuthal Equidistant
	ModifiedAzimuthalEquidistant::ModifiedAzimuthalEquidistant(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_origin"))}
		, _nuO{Nu(_a, _e, _latitudeOfOrigin)}
	{
	}

	auto ModifiedAzimuthalEquidistant::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const e2 = _e * _e;
		auto const dLambda = LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		auto const nu = Nu(_a, _e, phi);
		auto const psi = atan((1 - e2) * tan(phi) + e2 * _nuO * sin(_latitudeOfOrigin) / (nu * cos(phi)));
		auto const alpha = atan2(sin(dLambda), cos(_latitudeOfOrigin) * tan(psi) - sin(_latitudeOfOrigin) * cos(dLambda));
		auto const g = _e * sin(_latitudeOfOrigin) / sqrt(1 - e2);
		auto const h = _e * cos(_latitudeOfOrigin) * cos(alpha) / sqrt(1 - e2);
		auto const s = 0.0 == sin(alpha)
			? asin(cos(_latitudeOfOrigin) * sin(psi) - sin(_latitudeOfOrigin) * cos(psi)) * (cos(alpha) < 0 ? -1.0 : 1.0)
			: asin(sin(dLambda) * cos(psi) / sin(alpha));
		auto const h2 = h * h;
		auto const c = _nuO * s * ((1 - s * s * h2 * (1 - h2) / 6) + (pow(s, 3) / 8) * g * h * (1 - 2 * h2)
			+ (pow(s, 4) / 120) * (h2 * (4 - 7 * h2) - 3 * g * g * (1 - 7 * h2)) - (pow(s, 5) / 48) * g * h);
		x = _falseEasting + c * sin(alpha);
		y = _falseNorthing + c * cos(alpha);
	}

	auto ModifiedAzimuthalEquidistant::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const e2 = _e * _e;
		auto const dE = x - _falseEasting;
		auto const dN = y - _falseNorthing;
		auto const c = hypot(dE, dN);
		auto const alpha = atan2(dE, dN);
		auto const cosO = cos(_latitudeOfOrigin);
		auto const sinO = sin(_latitudeOfOrigin);
		auto const bigA = e2 * cosO * cosO * cos(alpha) * cos(alpha) / (1 - e2);
		auto const bigB = 3 * e2 * (1 - bigA) * sinO * cosO * cos(alpha) / (1 - e2);
		auto const d = c / _nuO;
		auto const j = d - bigA * (1 + bigA) * pow(d, 3) / 6 - bigB * (1 + 3 * bigA) * pow(d, 4) / 24;
		auto const k = 1 - bigA * j * j / 2 - bigB * pow(j, 3) / 6;
		auto const psi = asin(sinO * cos(j) + cosO * sin(j) * cos(alpha));
		phi = 0.0 == c ? _latitudeOfOrigin : atan((1 - e2 * k * sinO / sin(psi)) * tan(psi) / (1 - e2));
		lambda = _longitudeOfOrigin + asin(sin(alpha) * sin(j) / cos(psi));
		RefineReverse(x, y, lambda, phi);
	}

	auto ModifiedAzimuthalEquidistant::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<ModifiedAzimuthalEquidistant>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Guam Projection
	GuamProjection::GuamProjection(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_origin"))}
	{
		_mO = MeridianArc(_latitudeOfOrigin);
	}

	auto GuamProjection::MeridianArc(double phi) const -> double
	{
		auto const e2 = _e * _e;
		auto const e4 = e2 * e2;
		auto const e6 = e4 * e2;
		return _a * ((1 - e2 / 4 - 3 * e4 / 64 - 5 * e6 / 256) * phi
			- (3 * e2 / 8 + 3 * e4 / 32 + 45 * e6 / 1024) * sin(2 * phi)
			+ (15 * e4 / 256 + 45 * e6 / 1024) * sin(4 * phi)
			- (35 * e6 / 3072) * sin(6 * phi));
	}

	auto GuamProjection::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const w = sqrt(1 - _e * _e * sin(phi) * sin(phi));
		auto const dx = _a * LongitudeFromOrigin(lambda, _longitudeOfOrigin) * cos(phi) / w;
		x = _falseEasting + dx;
		y = _falseNorthing + MeridianArc(phi) - _mO + dx * dx * tan(phi) * w / (2 * _a);
	}

	auto GuamProjection::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const e2 = _e * _e;
		auto const e1 = (1 - sqrt(1 - e2)) / (1 + sqrt(1 - e2));
		auto const dE = x - _falseEasting;
		phi = _latitudeOfOrigin;
		for (auto i = 0; i < 3; ++i)
		{
			auto const m = _mO + (y - _falseNorthing) - dE * dE * tan(phi) * sqrt(1 - e2 * sin(phi) * sin(phi)) / (2 * _a);
			auto const mu = m / (_a * (1 - e2 / 4 - 3 * e2 * e2 / 64 - 5 * e2 * e2 * e2 / 256));
			phi = mu + (3 * e1 / 2 - 27 * pow(e1, 3) / 32) * sin(2 * mu) + (21 * e1 * e1 / 16 - 55 * pow(e1, 4) / 32) * sin(4 * mu)
				+ (151 * pow(e1, 3) / 96) * sin(6 * mu) + (1097 * pow(e1, 4) / 512) * sin(8 * mu);
		}
		lambda = _longitudeOfOrigin + dE * sqrt(1 - e2 * sin(phi) * sin(phi)) / (_a * cos(phi));
		RefineReverse(x, y, lambda, phi);
	}

	auto GuamProjection::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<GuamProjection>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Local Orthographic
	LocalOrthographic::LocalOrthographic(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _latitudeOfCentre{DEGREES_TO_RADIANS(Value("latitude_of_center"))}
		, _nuC{Nu(_a, _e, _latitudeOfCentre)}
		, _azimuth{DEGREES_TO_RADIANS(Value("azimuth"))}
		, _scale{Value("scale_factor")}
	{
	}

	auto LocalOrthographic::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const dLambda = LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		auto const nu = Nu(_a, _e, phi);
		auto const u = nu * cos(phi) * sin(dLambda);
		auto const v = nu * (sin(phi) * cos(_latitudeOfCentre) - cos(phi) * sin(_latitudeOfCentre) * cos(dLambda))
			+ _e * _e * (_nuC * sin(_latitudeOfCentre) - nu * sin(phi)) * cos(_latitudeOfCentre);
		x = _falseEasting + _scale * (u * cos(_azimuth) - v * sin(_azimuth));
		y = _falseNorthing + _scale * (u * sin(_azimuth) + v * cos(_azimuth));
	}

	auto LocalOrthographic::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		// Spherical first guess, then Newton on the forward conversion.
		auto const dE = (x - _falseEasting) / _scale;
		auto const dN = (y - _falseNorthing) / _scale;
		auto const u = dE * cos(_azimuth) + dN * sin(_azimuth);
		auto const v = -dE * sin(_azimuth) + dN * cos(_azimuth);
		auto const rho = hypot(u, v);
		auto const ratio = rho / _nuC;
		auto const c = asin(ratio > 1 ? 1 : ratio);
		phi = 0.0 == rho ? _latitudeOfCentre : asin(cos(c) * sin(_latitudeOfCentre) + v * sin(c) * cos(_latitudeOfCentre) / rho);
		lambda = _longitudeOfOrigin + atan2(u * sin(c), rho * cos(_latitudeOfCentre) * cos(c) - v * sin(_latitudeOfCentre) * sin(c));
		RefineReverse(x, y, lambda, phi);
	}

	auto LocalOrthographic::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LocalOrthographic>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion
}