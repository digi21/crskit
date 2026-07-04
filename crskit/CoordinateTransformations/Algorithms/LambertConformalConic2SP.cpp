#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "LambertConformalConic2SP.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	LambertConformalConic2SP::LambertConformalConic2SP(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _falseOriginLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_origin").GetValue()))}
		, _falseOriginLongitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("central_meridian").GetValue()))}
		, _firstStandardParallelLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("standard_parallel1").GetValue()))}
		, _secondStandardParallelLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("standard_parallel2").GetValue()))}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		auto const f = (_semiMajorAxis - _semiMinorAxis) / _semiMajorAxis;
		_e = sqrt(2 * f - f * f);
		auto const e2 = 2 * f - f * f;

		_m1 = cos(_firstStandardParallelLatitude) / sqrt(1.0 - e2 * pow(sin(_firstStandardParallelLatitude), 2.0));
		_m2 = cos(_secondStandardParallelLatitude) / sqrt(1.0 - e2 * pow(sin(_secondStandardParallelLatitude), 2.0));

		_t1 = tan(M_PI / 4.0 - _firstStandardParallelLatitude / 2.0) / pow((1.0 - _e * sin(_firstStandardParallelLatitude)) / (1.0 + _e * sin(_firstStandardParallelLatitude)), _e / 2.0);
		_t2 = tan(M_PI / 4.0 - _secondStandardParallelLatitude / 2.0) / pow((1.0 - _e * sin(_secondStandardParallelLatitude)) / (1.0 + _e * sin(_secondStandardParallelLatitude)), _e / 2.0);
		_tf = tan(M_PI / 4.0 - _falseOriginLatitude / 2.0) / pow((1.0 - _e * sin(_falseOriginLatitude)) / (1.0 + _e * sin(_falseOriginLatitude)), _e / 2.0);

		_n = (log(_m1) - log(_m2)) / (log(_t1) - log(_t2));
		_f = _m1 / (_n * pow(_t1, _n));
		_rf = _semiMajorAxis * _f * pow(_tf, _n);
	}

#pragma region MapProjectionBase members
	auto LambertConformalConic2SP::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const sinPhi = sin(phi);

		auto const t = tan(M_PI / 4.0 - phi / 2.0) / pow((1.0 - _e * sinPhi) / (1.0 + _e * sinPhi), _e / 2.0);
		auto const r = _semiMajorAxis * _f * pow(t, _n);
		auto const theta = _n * (lambda - _falseOriginLongitude);

		x = _falseEasting + r * sin(theta);
		y = _falseNorthing + _rf - r * cos(theta);
	}

	auto LambertConformalConic2SP::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const r = (_n >= 0 ? 1.0 : -1.0) * sqrt(pow(x - _falseEasting, 2.0) + pow(_rf - (y - _falseNorthing), 2.0));
		auto const t = pow(r / (_semiMajorAxis * _f), 1.0 / _n);
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

		auto const theta = atan((x - _falseEasting) / (_rf - (y - _falseNorthing)));
		lambda = theta / _n + _falseOriginLongitude;
	}
#pragma endregion
#pragma region IMathTransform members
	auto LambertConformalConic2SP::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<LambertConformalConic2SP>(_parameters, !_inverse);

		return _inverseTransformation;
	}
#pragma endregion
}
