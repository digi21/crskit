#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "LambertConformalConic1SP.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	LambertConformalConic1SP::LambertConformalConic1SP(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _latitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_origin").GetValue()))}
		, _longitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("central_meridian").GetValue()))}
		, _scaleFactorAtNaturalOrigin{std::get<double>(parameters->GetParameter("scale_factor").GetValue())}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		auto const f = (_semiMajorAxis - _semiMinorAxis) / _semiMajorAxis;
		_e = sqrt(2 * f - f * f);
		auto const e2 = 2 * f - f * f;

		_m0 = cos(_latitudeOfNaturalOrigin) / sqrt(1.0 - e2 * pow(sin(_latitudeOfNaturalOrigin), 2.0));
		_t0 = tan(M_PI / 4.0 - _latitudeOfNaturalOrigin / 2.0) / pow((1.0 - _e * sin(_latitudeOfNaturalOrigin)) / (1.0 + _e * sin(_latitudeOfNaturalOrigin)), _e / 2.0);
		_n = sin(_latitudeOfNaturalOrigin);
		_f = _m0 / (_n * pow(_t0, _n));
		_r0 = _semiMajorAxis * _f * pow(_t0, _n) * _scaleFactorAtNaturalOrigin;
	}
#pragma endregion
#pragma region MapProjectionBase members
	auto LambertConformalConic1SP::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const t = tan(M_PI / 4.0 - phi / 2.0) / pow((1.0 - _e * sin(phi)) / (1.0 + _e * sin(phi)), _e / 2.0);
		auto const R = _semiMajorAxis * _f * pow(t, _n) * _scaleFactorAtNaturalOrigin;

		auto const theta = _n * (lambda - _longitudeOfNaturalOrigin);

		x = _falseEasting + R * sin(theta);
		y = _falseNorthing + _r0 - R * cos(theta);
	}

	auto LambertConformalConic1SP::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const r = (_n >= 0 ? 1.0 : -1.0) * sqrt(pow(x - _falseEasting, 2.0) + pow(_r0 - (y - _falseNorthing), 2.0));
		auto const t = pow(r / (_semiMajorAxis * _scaleFactorAtNaturalOrigin * _f), 1.0 / _n);

		phi = M_PI / 2.0 * atan(t);
		double diferencia;
		int iteration = 0;

		do
		{
			auto const newLatitude = M_PI / 2.0 - 2.0 * atan(t * pow((1.0 - _e * sin(phi)) / (1 + _e * sin(phi)), _e / 2.0));;
			diferencia = fabs(newLatitude - phi);
			phi = newLatitude;
		}
		while (diferencia > EPSILON && ++iteration < 50); // cap: avoid an infinite loop on non-convergent / NaN input

		auto const theta = atan((x - _falseEasting) / (_r0 - (y - _falseNorthing)));
		lambda = theta / _n + _longitudeOfNaturalOrigin;
	}
#pragma endregion
#pragma region IMathTransform members
	auto LambertConformalConic1SP::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<LambertConformalConic1SP>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
