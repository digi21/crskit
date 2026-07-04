#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "LambertAzimuthalEqualArea.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	LambertAzimuthalEqualArea::LambertAzimuthalEqualArea(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _projectionCenterLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_center").GetValue()))}
		, _projectionCenterLongitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("longitude_of_center").GetValue()))}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		auto const f = (_semiMajorAxis - _semiMinorAxis) / _semiMajorAxis;
		_e = sqrt(2 * f - f * f);

		_latitude0 = _projectionCenterLatitude;
		_longitude0 = _projectionCenterLongitude;
		_sinlat0 = sin(_latitude0);
		_q0 = (1 - pow(_e, 2)) * (_sinlat0 / (1 - pow(_e, 2) * pow(_sinlat0, 2)) - (1 / (2 * _e)) * log((1 - _e * _sinlat0) / (1 + _e * _sinlat0)));
		_qp = (1 - pow(_e, 2)) * (1 / (1 - pow(_e, 2)) - (1 / (2 * _e)) * (log((1 - _e) / (1 + _e))));
		_beta0 = asin(_q0 / _qp);
		_rq = _semiMajorAxis * sqrt(_qp / 2);
		_d = _semiMajorAxis * (cos(_latitude0) / sqrt(1 - pow(_e, 2) * pow(_sinlat0, 2))) / (_rq * cos(_beta0));
	}

#pragma region MapProjectionBase members
	auto LambertAzimuthalEqualArea::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const sinlat = sin(phi);
		auto const q = (1 - pow(_e, 2)) * (sinlat / (1 - pow(_e, 2) * pow(sinlat, 2)) - (1 / (2 * _e)) * log((1 - _e * sinlat) / (1 + _e * sinlat)));
		auto const beta = asin(clampToUnit(q / _qp));
		// Near the antipode the denominator -> 0 (then 2/denominator -> +inf, fine) or slightly negative
		// by rounding; guard the sqrt so an out-of-domain point yields a finite result, never NaN.
		auto const radicand = 2 / (1 + sin(_beta0) * sin(beta) + cos(_beta0) * cos(beta) * cos(lambda - _longitude0));
		auto const B = _rq * (radicand > 0 ? sqrt(radicand) : 0.0);

		if (fabs(_latitude0) == M_PI / 2)
		{
			if (_latitude0 > 0)
			{
				auto const p = _semiMajorAxis * sqrt(_qp - q);
				x = _falseEasting + (p * sin(lambda - _longitude0));
				y = _falseNorthing - (p * cos(lambda - _longitude0));
			}
			else
			{
				auto const p = _semiMajorAxis * sqrt(_qp + q);
				x = _falseEasting + (p * sin(lambda - _longitude0));
				y = _falseNorthing + (p * cos(lambda - _longitude0));
			}
		}
		else
		{
			x = _falseEasting + (B * _d) * (cos(beta) * sin(lambda - _longitude0));
			y = _falseNorthing + (B / _d) * ((cos(_beta0) * sin(beta)) - (sin(_beta0) * cos(beta) * cos(lambda - _longitude0)));
		}
	}

	auto LambertAzimuthalEqualArea::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const p = sqrt(pow((x - _falseEasting) / _d, 2) + (pow(_d * (y - _falseNorthing), 2)));
		auto const C = 2 * asin(clampToUnit(p / (2 * _rq)));

		auto const betaPrime = asin(clampToUnit(cos(C) * sin(_beta0) + _d * (y - _falseNorthing) * sin(C) * cos(_beta0) / p));
		lambda = _longitude0 + atan((x - _falseEasting) * sin(C) / (_d * p * cos(_beta0) * cos(C) - pow(_d, 2) * (y - _falseNorthing) * sin(_beta0) * sin(C)));

		phi = betaPrime + ((pow(_e, 2) / 3 + 31 * pow(_e, 4) / 180 + 517 * pow(_e, 6) / 5040) * sin(2 * betaPrime))
		+ (23 * pow(_e, 4) / 360 + 251 * pow(_e, 6) / 3780) * sin(4 * betaPrime)
		+ (761 * pow(_e, 6) / 45360) * sin(6 * betaPrime);
	}
#pragma endregion

#pragma region IMathTransform members
	auto LambertAzimuthalEqualArea::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<LambertAzimuthalEqualArea>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
