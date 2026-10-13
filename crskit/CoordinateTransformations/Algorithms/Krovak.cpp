#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "Krovak.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	namespace
	{
		auto Value(std::shared_ptr<IProjection> const& parameters, std::string const& name) -> double
		{
			return std::get<double>(parameters->GetParameter(name).GetValue());
		}
	}

	Krovak::Krovak(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: Krovak{parameters, inverse, false}
	{
	}

	Krovak::Krovak(std::shared_ptr<IProjection> const& parameters, bool inverse, bool modified)
		: MapProjectionBase{parameters, inverse}
		, _modified{modified}
	{
		auto const a = Value(parameters, "semi_major");
		auto const f = (a - Value(parameters, "semi_minor")) / a;
		_e = sqrt(2 * f - f * f);
		auto const e2 = _e * _e;

		auto const latC = DEGREES_TO_RADIANS(Value(parameters, "latitude_of_center"));
		_longitudeOfOrigin = DEGREES_TO_RADIANS(Value(parameters, "longitude_of_center"));
		_alphaC = DEGREES_TO_RADIANS(Value(parameters, "azimuth"));
		_latitudeP = DEGREES_TO_RADIANS(Value(parameters, "pseudo_standard_parallel_1"));
		auto const kP = Value(parameters, "scale_factor");
		_falseEasting = Value(parameters, "false_easting");
		_falseNorthing = Value(parameters, "false_northing");

		auto const sinC = sin(latC);
		auto const bigA = a * sqrt(1 - e2) / (1 - e2 * sinC * sinC);
		_b = sqrt(1 + e2 * pow(cos(latC), 4) / (1 - e2));
		auto const gamma0 = asin(sinC / _b);
		_t0 = tan(M_PI / 4 + gamma0 / 2) * pow((1 + _e * sinC) / (1 - _e * sinC), _e * _b / 2) / pow(tan(M_PI / 4 + latC / 2), _b);
		_n = sin(_latitudeP);
		_r0 = kP * bigA / tan(_latitudeP);

		if (modified)
		{
			_x0 = Value(parameters, "evaluation_point_ordinate_1");
			_y0 = Value(parameters, "evaluation_point_ordinate_2");
			for (auto i = 0u; i < _c.size(); ++i)
				_c[i] = Value(parameters, std::format("C{}", i + 1));
		}
	}

	auto Krovak::Correction(double xr, double yr, double& dx, double& dy) const -> void
	{
		auto const& c = _c;
		auto const x2 = xr * xr;
		auto const y2 = yr * yr;
		dx = c[0] + c[2] * xr - c[3] * yr - 2 * c[5] * xr * yr + c[4] * (x2 - y2) + c[6] * xr * (x2 - 3 * y2) - c[7] * yr * (3 * x2 - y2)
			+ 4 * c[8] * xr * yr * (x2 - y2) + c[9] * (x2 * x2 + y2 * y2 - 6 * x2 * y2);
		dy = c[1] + c[2] * yr + c[3] * xr + 2 * c[4] * xr * yr + c[5] * (x2 - y2) + c[7] * xr * (x2 - 3 * y2) + c[6] * yr * (3 * x2 - y2)
			- 4 * c[9] * xr * yr * (x2 - y2) + c[8] * (x2 * x2 + y2 * y2 - 6 * x2 * y2);
	}

	// (x, y) = (southing, westing).
	auto Krovak::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const sinPhi = sin(phi);
		auto const u = 2 * (atan(_t0 * pow(tan(phi / 2 + M_PI / 4), _b) / pow((1 + _e * sinPhi) / (1 - _e * sinPhi), _e * _b / 2)) - M_PI / 4);
		auto const v = -_b * LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		auto const t = asin(cos(_alphaC) * sin(u) + sin(_alphaC) * cos(u) * cos(v));
		// The atan2 form of D, valid beyond 90 deg of pseudo-longitude.
		auto const d = atan2(cos(u) * sin(v) / cos(t), (cos(_alphaC) * sin(t) - sin(u)) / (sin(_alphaC) * cos(t)));
		auto const theta = _n * d;
		auto const r = _r0 * pow(tan(M_PI / 4 + _latitudeP / 2), _n) / pow(tan(t / 2 + M_PI / 4), _n);
		auto const xp = r * cos(theta);
		auto const yp = r * sin(theta);

		x = _falseNorthing + xp;
		y = _falseEasting + yp;
		if (_modified)
		{
			double dx, dy;
			Correction(xp - _x0, yp - _y0, dx, dy);
			x -= dx;
			y -= dy;
		}
	}

	auto Krovak::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto xp = x - _falseNorthing;
		auto yp = y - _falseEasting;
		if (_modified)
		{
			double dx, dy;
			Correction(xp - _x0, yp - _y0, dx, dy);
			xp += dx;
			yp += dy;
		}

		auto const r = hypot(xp, yp);
		auto const theta = atan2(yp, xp);
		auto const d = theta / _n;
		auto const t = 2 * (atan(pow(_r0 / r, 1 / _n) * tan(M_PI / 4 + _latitudeP / 2)) - M_PI / 4);
		auto const u = asin(cos(_alphaC) * sin(t) - sin(_alphaC) * cos(t) * cos(d));
		auto const v = asin(cos(t) * sin(d) / cos(u));

		phi = u;
		for (auto i = 0; i < 20; ++i)
		{
			auto const sinPhi = sin(phi);
			auto const next = 2 * (atan(pow(_t0, -1 / _b) * pow(tan(u / 2 + M_PI / 4), 1 / _b) * pow((1 + _e * sinPhi) / (1 - _e * sinPhi), _e / 2)) - M_PI / 4);
			auto const delta = fabs(next - phi);
			phi = next;
			if (delta < 1e-14)
				break;
		}
		lambda = _longitudeOfOrigin - v / _b;
	}

	auto Krovak::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<Krovak>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	auto KrovakNorthOrientated::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<KrovakNorthOrientated>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	KrovakModified::KrovakModified(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: Krovak{parameters, inverse, true}
	{
	}

	auto KrovakModified::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<KrovakModified>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	auto KrovakModifiedNorthOrientated::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<KrovakModifiedNorthOrientated>(_parameters, !_inverse);
		return _inverseTransformation;
	}
}