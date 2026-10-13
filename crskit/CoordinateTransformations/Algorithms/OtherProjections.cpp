#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "../../CoordinateSystems/Projection.h"
#include "MapProjectionBase.h"
#include "OtherProjections.h"

#include <complex>

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
	}

#pragma region Meridian arc
	auto MeridianArcSeries::M(double phi) const -> double
	{
		auto const e2 = _e * _e;
		auto const e4 = e2 * e2;
		auto const e6 = e4 * e2;
		return _a * ((1 - e2 / 4 - 3 * e4 / 64 - 5 * e6 / 256) * phi
			- (3 * e2 / 8 + 3 * e4 / 32 + 45 * e6 / 1024) * sin(2 * phi)
			+ (15 * e4 / 256 + 45 * e6 / 1024) * sin(4 * phi)
			- (35 * e6 / 3072) * sin(6 * phi));
	}

	auto MeridianArcSeries::Footpoint(double m) const -> double
	{
		auto const e2 = _e * _e;
		auto const mu = m / (_a * (1 - e2 / 4 - 3 * e2 * e2 / 64 - 5 * e2 * e2 * e2 / 256));
		auto const e1 = (1 - sqrt(1 - e2)) / (1 + sqrt(1 - e2));
		return mu + (3 * e1 / 2 - 27 * pow(e1, 3) / 32) * sin(2 * mu) + (21 * e1 * e1 / 16 - 55 * pow(e1, 4) / 32) * sin(4 * mu)
			+ (151 * pow(e1, 3) / 96) * sin(6 * mu) + (1097 * pow(e1, 4) / 512) * sin(8 * mu);
	}
#pragma endregion

#pragma region Bonne (South Orientated)
	BonneSouthOrientated::BonneSouthOrientated(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _arc{_a, _e}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_origin"))}
	{
		_apex = _a * cos(_latitudeOfOrigin) / sqrt(1 - _e * _e * sin(_latitudeOfOrigin) * sin(_latitudeOfOrigin)) / sin(_latitudeOfOrigin);
		_mO = _arc.M(_latitudeOfOrigin);
	}

	auto BonneSouthOrientated::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const m = cos(phi) / sqrt(1 - _e * _e * sin(phi) * sin(phi));
		auto const rho = _apex + _mO - _arc.M(phi);
		auto const t = _a * m * LongitudeFromOrigin(lambda, _longitudeOfOrigin) / rho;
		x = _falseEasting - rho * sin(t);
		y = _falseNorthing - (_apex - rho * cos(t));
	}

	auto BonneSouthOrientated::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const dX = _falseEasting - x;
		auto const dY = _falseNorthing - y;
		auto const sign = _latitudeOfOrigin < 0 ? -1.0 : 1.0;
		auto const rho = sign * hypot(dX, _apex - dY);
		phi = _arc.Footpoint(_apex + _mO - rho);
		auto const m = cos(phi) / sqrt(1 - _e * _e * sin(phi) * sin(phi));
		lambda = fabs(m) < 1e-15 ? _longitudeOfOrigin : _longitudeOfOrigin + rho * atan2(sign * dX, sign * (_apex - dY)) / (_a * m);
		RefineReverse(x, y, lambda, phi);
	}

	auto BonneSouthOrientated::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<BonneSouthOrientated>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Hyperbolic Cassini-Soldner
	HyperbolicCassiniSoldner::HyperbolicCassiniSoldner(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _arc{_a, _e}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_origin"))}
		, _mO{_arc.M(_latitudeOfOrigin)}
	{
	}

	auto HyperbolicCassiniSoldner::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const e2 = _e * _e;
		auto const nu = Nu(_a, _e, phi);
		auto const c = e2 * cos(phi) * cos(phi) / (1 - e2);
		auto const t = tan(phi) * tan(phi);
		auto const a = LongitudeFromOrigin(lambda, _longitudeOfOrigin) * cos(phi);
		auto const bigX = _arc.M(phi) - _mO + nu * tan(phi) * (a * a / 2 + (5 - t + 6 * c) * pow(a, 4) / 24);
		x = _falseEasting + nu * (a - t * pow(a, 3) / 6 - (8 - t + 8 * c) * t * pow(a, 5) / 120);
		y = _falseNorthing + bigX - pow(bigX, 3) / (6 * Rho(_a, _e, phi) * nu);
	}

	auto HyperbolicCassiniSoldner::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		phi = _arc.Footpoint(_mO + (y - _falseNorthing));
		lambda = _longitudeOfOrigin + (x - _falseEasting) / (Nu(_a, _e, phi) * cos(phi));
		RefineReverse(x, y, lambda, phi);
	}

	auto HyperbolicCassiniSoldner::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<HyperbolicCassiniSoldner>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Lambert Conic Near-Conformal
	LambertConicNearConformal::LambertConicNearConformal(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_origin"))}
		, _k0{Value("scale_factor")}
	{
		auto const f = 1 - sqrt(1 - _e * _e);
		auto const n = f / (2 - f);
		auto const n2 = n * n, n3 = n2 * n, n4 = n3 * n, n5 = n4 * n;
		_coefficients[0] = _a * (1 - n + 5 * (n2 - n3) / 4 + 81 * (n4 - n5) / 64) * M_PI / 180;
		_coefficients[1] = 3 * _a * (n - n2 + 7 * (n3 - n4) / 8 + 55 * n5 / 64) / 2;
		_coefficients[2] = 15 * _a * (n2 - n3 + 3 * (n4 - n5) / 4) / 16;
		_coefficients[3] = 35 * _a * (n3 - n4 + 11 * n5 / 16) / 48;
		_coefficients[4] = 315 * _a * (n4 - n5) / 512;

		auto const nuO = Nu(_a, _e, _latitudeOfOrigin);
		_bigA = 1 / (6 * Rho(_a, _e, _latitudeOfOrigin) * nuO);
		_r0 = _k0 * nuO / tan(_latitudeOfOrigin);
		_s0 = S(_latitudeOfOrigin);
	}

	auto LambertConicNearConformal::S(double phi) const -> double
	{
		auto const& c = _coefficients;
		return c[0] * RADIANS_TO_DEGREES(phi) - c[1] * sin(2 * phi) + c[2] * sin(4 * phi) - c[3] * sin(6 * phi) + c[4] * sin(8 * phi);
	}

	auto LambertConicNearConformal::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const m = S(phi) - _s0;
		auto const bigM = _k0 * (m + _bigA * m * m * m);
		auto const r = _r0 - bigM;
		auto const theta = LongitudeFromOrigin(lambda, _longitudeOfOrigin) * sin(_latitudeOfOrigin);
		x = _falseEasting + r * sin(theta);
		y = _falseNorthing + bigM + r * sin(theta) * tan(theta / 2);
	}

	auto LambertConicNearConformal::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const sign = _latitudeOfOrigin < 0 ? -1.0 : 1.0;
		auto const dE = x - _falseEasting;
		auto const dN = _r0 - (y - _falseNorthing);
		auto const theta = atan2(sign * dE, sign * dN);
		auto const bigM = _r0 - sign * hypot(dE, dN);
		auto m = bigM;
		for (auto i = 0; i < 10; ++i)
			m -= (bigM - _k0 * m - _k0 * _bigA * m * m * m) / (-_k0 - 3 * _k0 * _bigA * m * m);
		phi = _latitudeOfOrigin + DEGREES_TO_RADIANS(m / _coefficients[0]);
		for (auto i = 0; i < 10; ++i)
		{
			auto const& c = _coefficients;
			auto const ds = c[0] * 180 / M_PI - 2 * c[1] * cos(2 * phi) + 4 * c[2] * cos(4 * phi) - 6 * c[3] * cos(6 * phi) + 8 * c[4] * cos(8 * phi);
			phi += (m + _s0 - S(phi)) / ds;
		}
		lambda = _longitudeOfOrigin + theta / sin(_latitudeOfOrigin);
		RefineReverse(x, y, lambda, phi);
	}

	auto LambertConicNearConformal::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertConicNearConformal>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Transverse Mercator Zoned Grid System
	TransverseMercatorZonedGrid::TransverseMercatorZonedGrid(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		// Rounded to 1e-9 deg: they arrive through a degree conversion (6 may be 5.999...), and the zone boundaries are
		// multiples of them.
		, _initialLongitude{std::round(std::get<double>(parameters->GetParameter("initial_longitude").GetValue()) * 1e9) / 1e9}
		, _zoneWidth{std::round(std::get<double>(parameters->GetParameter("zone_width").GetValue()) * 1e9) / 1e9}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
	{
		std::vector<Parameter> values{
			{ "semi_major", parameters->GetParameter("semi_major").GetValue() },
			{ "semi_minor", parameters->GetParameter("semi_minor").GetValue() },
			{ "latitude_of_origin", parameters->GetParameter("latitude_of_origin").GetValue() },
			{ "central_meridian", 0.0 },
			{ "scale_factor", parameters->GetParameter("scale_factor").GetValue() },
			{ "false_easting", 0.0 },
			{ "false_northing", parameters->GetParameter("false_northing").GetValue() } };
		_transverseMercator = make_shared<TransverseMercator>(make_shared<Projection>("Transverse_Mercator", values), false);
	}

	auto TransverseMercatorZonedGrid::Zone(double longitudeDegrees) const -> int
	{
		// A point on a zone boundary belongs to the zone to its east.
		auto const zones = std::lround(360.0 / _zoneWidth);
		auto const index = static_cast<long long>(std::floor((longitudeDegrees - _initialLongitude) / _zoneWidth));
		return static_cast<int>(((index % zones) + zones) % zones) + 1;
	}

	auto TransverseMercatorZonedGrid::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const zone = Zone(RADIANS_TO_DEGREES(lambda));
		auto const centralMeridian = DEGREES_TO_RADIANS(_initialLongitude + zone * _zoneWidth - _zoneWidth / 2);
		double easting;
		_transverseMercator->Geo2Proj(lambda - centralMeridian, phi, easting, y);
		x = zone * 1e6 + _falseEasting + easting;
	}

	auto TransverseMercatorZonedGrid::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		// |E'| stays below 500 km, so the millions of the easting are the zone.
		auto const zone = static_cast<int>(std::lround((x - _falseEasting) / 1e6));
		auto const centralMeridian = DEGREES_TO_RADIANS(_initialLongitude + zone * _zoneWidth - _zoneWidth / 2);
		_transverseMercator->Proj2Geo(x - _falseEasting - zone * 1e6, y, lambda, phi);
		lambda += centralMeridian;
	}

	auto TransverseMercatorZonedGrid::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<TransverseMercatorZonedGrid>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region New Zealand Map Grid
	namespace
	{
		constexpr double kNzmgA[] = { 0.6399175073, -0.1358797613, 0.063294409, -0.02526853, 0.0117879, -0.0055161, 0.0026906, -0.001333, 0.00067, -0.00034 };
		std::complex<double> const kNzmgB[] = { { 0.7557853228, 0.0 }, { 0.249204646, 0.003371507 }, { -0.001541739, 0.041058560 },
			{ -0.10162907, 0.01727609 }, { -0.26623489, -0.36249218 }, { -0.6870983, -1.1651967 } };
		std::complex<double> const kNzmgC[] = { { 1.3231270439, 0.0 }, { -0.577245789, -0.007809598 }, { 0.508307513, -0.112208952 },
			{ -0.15094762, 0.18200602 }, { 1.01418179, 1.64497696 }, { 1.9660549, 2.5127645 } };
		constexpr double kNzmgD[] = { 1.5627014243, 0.5185406398, -0.03333098, -0.1052906, -0.0368594, 0.007317, 0.01220, 0.00394, -0.0013 };
	}

	NewZealandMapGrid::NewZealandMapGrid(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _latitudeOfOrigin{DEGREES_TO_RADIANS(Value("latitude_of_origin"))}
	{
	}

	auto NewZealandMapGrid::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		// Latitude difference in units of 1e5 arc-seconds.
		auto const dPhi = RADIANS_TO_DEGREES(phi - _latitudeOfOrigin) * 3600 * 1e-5;
		auto dPsi = 0.0;
		for (auto i = 9; i >= 0; --i)
			dPsi = (dPsi + kNzmgA[i]) * dPhi;
		std::complex<double> const zeta{ dPsi, LongitudeFromOrigin(lambda, _longitudeOfOrigin) };
		std::complex<double> z{};
		for (auto i = 5; i >= 0; --i)
			z = (z + kNzmgB[i]) * zeta;
		y = _falseNorthing + _a * z.real();
		x = _falseEasting + _a * z.imag();
	}

	auto NewZealandMapGrid::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		std::complex<double> const z{ (y - _falseNorthing) / _a, (x - _falseEasting) / _a };
		std::complex<double> zeta{};
		for (auto i = 5; i >= 0; --i)
			zeta = (zeta + kNzmgC[i]) * z;
		for (auto iteration = 0; iteration < 4; ++iteration)
		{
			std::complex<double> numerator = z, denominator{};
			auto power = std::complex<double>{ 1.0, 0.0 };
			for (auto i = 0; i < 6; ++i)
			{
				denominator += static_cast<double>(i + 1) * kNzmgB[i] * power;
				power *= zeta;
				numerator += static_cast<double>(i) * kNzmgB[i] * power;
			}
			zeta = numerator / denominator;
		}
		auto dPhi = 0.0;
		for (auto i = 8; i >= 0; --i)
			dPhi = (dPhi + kNzmgD[i]) * zeta.real();
		phi = _latitudeOfOrigin + DEGREES_TO_RADIANS(dPhi * 1e5 / 3600);
		lambda = _longitudeOfOrigin + zeta.imag();
		RefineReverse(x, y, lambda, phi);
	}

	auto NewZealandMapGrid::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<NewZealandMapGrid>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Tunisia Mining Grid
	namespace
	{
		constexpr double kGradsPerRadian = 200 / M_PI;
		constexpr double kLongitudeGradsPerKilometre = 0.012185;

		// Latitude grads per kilometre of northing, north and south of the origin.
		auto LatitudeGradsPerKilometre(bool north) -> double
		{
			return north ? 0.010015 : 0.01002;
		}
	}

	TunisiaMiningGrid::TunisiaMiningGrid(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
		, _latitudeOfOrigin{Value("latitude_of_origin") / 0.9}
	{
	}

	auto TunisiaMiningGrid::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const latitude = phi * kGradsPerRadian;
		auto const longitude = LongitudeFromOrigin(lambda, _longitudeOfOrigin) * kGradsPerRadian;
		x = _falseEasting + 1000 * longitude / kLongitudeGradsPerKilometre;
		y = _falseNorthing + 1000 * (latitude - _latitudeOfOrigin) / LatitudeGradsPerKilometre(latitude > _latitudeOfOrigin);
	}

	auto TunisiaMiningGrid::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const northKm = (y - _falseNorthing) / 1000;
		phi = (_latitudeOfOrigin + northKm * LatitudeGradsPerKilometre(northKm > 0)) / kGradsPerRadian;
		lambda = _longitudeOfOrigin + (x - _falseEasting) / 1000 * kLongitudeGradsPerKilometre / kGradsPerRadian;
	}

	auto TunisiaMiningGrid::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<TunisiaMiningGrid>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion

#pragma region Laborde Oblique Mercator
	LabordeObliqueMercator::LabordeObliqueMercator(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: SimpleProjection{parameters, inverse}
	{
		auto const e2 = _e * _e;
		auto const phiC = DEGREES_TO_RADIANS(Value("latitude_of_center"));
		auto const alphaC = DEGREES_TO_RADIANS(Value("azimuth"));
		_b = sqrt(1 + e2 * pow(cos(phiC), 4) / (1 - e2));
		_phiS = asin(sin(phiC) / _b);
		_r = _a * Value("scale_factor") * sqrt(1 - e2) / (1 - e2 * sin(phiC) * sin(phiC));
		_c = log(tan(M_PI / 4 + _phiS / 2)) - _b * IsometricLatitude(phiC);
		_g = std::complex<double>{ 1 - cos(2 * alphaC), sin(2 * alphaC) } / 12.0;
	}

	auto LabordeObliqueMercator::IsometricLatitude(double phi) const -> double
	{
		return log(tan(M_PI / 4 + phi / 2) * pow((1 - _e * sin(phi)) / (1 + _e * sin(phi)), _e / 2));
	}

	auto LabordeObliqueMercator::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const l = _b * LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		auto const q = _c + _b * IsometricLatitude(phi);
		auto const p = 2 * atan(exp(q)) - M_PI / 2;
		auto const u = cos(p) * cos(l) * cos(_phiS) + sin(p) * sin(_phiS);
		auto const v = cos(p) * cos(l) * sin(_phiS) - sin(p) * cos(_phiS);
		auto const w = cos(p) * sin(l);
		auto const d = hypot(u, v);
		auto const lPrime = 0.0 == d ? 0.0 : 2 * atan(v / (u + d));
		auto const pPrime = 0.0 == d ? (w < 0 ? -M_PI / 2 : M_PI / 2) : atan(w / d);
		std::complex<double> const h{ -lPrime, log(tan(M_PI / 4 + pPrime / 2)) };
		auto const result = h + _g * h * h * h;
		x = _falseEasting + _r * result.imag();
		y = _falseNorthing + _r * result.real();
	}

	auto LabordeObliqueMercator::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		std::complex<double> const h0{ (y - _falseNorthing) / _r, (x - _falseEasting) / _r };
		auto h = h0;
		for (auto i = 0; i < 20; ++i)
		{
			auto const next = (h0 + 2.0 * _g * h * h * h) / (3.0 * _g * h * h + 1.0);
			auto const delta = std::abs(next - h);
			h = next;
			if (delta < 1e-14)
				break;
		}
		auto const lPrime = -h.real();
		auto const pPrime = 2 * atan(exp(h.imag())) - M_PI / 2;
		auto const u = cos(pPrime) * cos(lPrime) * cos(_phiS) + cos(pPrime) * sin(lPrime) * sin(_phiS);
		auto const v = sin(pPrime);
		auto const w = cos(pPrime) * cos(lPrime) * sin(_phiS) - cos(pPrime) * sin(lPrime) * cos(_phiS);
		auto const d = hypot(u, v);
		auto const l = 0.0 == d ? 0.0 : 2 * atan(v / (u + d));
		auto const p = 0.0 == d ? (w < 0 ? -M_PI / 2 : M_PI / 2) : atan(w / d);
		lambda = _longitudeOfOrigin + l / _b;
		auto const q = (log(tan(M_PI / 4 + p / 2)) - _c) / _b;
		phi = 2 * atan(exp(q)) - M_PI / 2;
		for (auto i = 0; i < 20; ++i)
		{
			auto const next = 2 * atan(pow((1 + _e * sin(phi)) / (1 - _e * sin(phi)), _e / 2) * exp(q)) - M_PI / 2;
			auto const delta = fabs(next - phi);
			phi = next;
			if (delta < 1e-14)
				break;
		}
		RefineReverse(x, y, lambda, phi);
	}

	auto LabordeObliqueMercator::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LabordeObliqueMercator>(_parameters, !_inverse);
		return _inverseTransformation;
	}
#pragma endregion
}