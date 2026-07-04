#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "ObliqueMercator.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	ObliqueMercator::ObliqueMercator(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _projectionCenterLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_center").GetValue()))}
		, _projectionCenterLongitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("longitude_of_center").GetValue()))}
		, _initialLineAzimuth{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("azimuth").GetValue()))}
		, _rectifiedGridAngle{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("rectified_grid_angle").GetValue()))}
		, _scaleAtCenter{std::get<double>(parameters->GetParameter("scale_factor").GetValue())}
		, _eastingAtProjectionCenter{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _northingAtProjectionCenter{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		auto const f = (_semiMajorAxis - _semiMinorAxis) / _semiMajorAxis;

		_e = sqrt(2 * f - pow(f, 2));
		_b = sqrt(1 + (pow(_e, 2) * pow(cos(_projectionCenterLatitude), 4) / (1 - pow(_e, 2))));
		_a = _semiMajorAxis * _b * _scaleAtCenter * sqrt(1 - pow(_e, 2)) / (1 - pow(_e, 2) * pow(sin(_projectionCenterLatitude), 2));
		_t0 = tan(M_PI / 4 - _projectionCenterLatitude / 2) / pow(((1 - _e * sin(_projectionCenterLatitude)) / (1 + _e * sin(_projectionCenterLatitude))), _e / 2);
		_d = _b * sqrt(1 - pow(_e, 2)) / (cos(_projectionCenterLatitude) * sqrt(1 - pow(_e, 2) * pow(sin(_projectionCenterLatitude), 2)));

		if (_d < 1)
			_f = _d * SIGN(_projectionCenterLatitude);
		else
			_f = _d + sqrt(pow(_d, 2) - 1) * SIGN(_projectionCenterLatitude);

		_h = _f * pow(_t0, _b);
		_g = (_f - 1 / _f) / 2;
		_gamma0 = asin(sin(_initialLineAzimuth) / _d);
		_longitude0 = _projectionCenterLongitude - asin(_g * tan(_gamma0)) / _b;

		_vc = 0.0;

		if (fabs(_initialLineAzimuth - M_PI / 2) < 1E-10)
			_uc = _a * (_projectionCenterLongitude - _longitude0);
		else
			_uc = (_a / _b) * atan(sqrt(pow(_d, 2) - 1) / cos(_initialLineAzimuth)) * SIGN(_projectionCenterLatitude);
	}

#pragma region MapProjectionBase members
	auto ObliqueMercator::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const sinPhi = sin(phi);

		auto const t = tan(M_PI / 4 - phi / 2) / pow(((1 - _e * sinPhi) / (1 + _e * sinPhi)), _e / 2);
		auto const Q = _h / pow(t, _b);
		auto const S = (Q - 1 / Q) / 2;
		auto const T = (Q + 1 / Q) / 2;
		auto const V = sin(_b * (lambda - _longitude0));
		auto const U = (-V * cos(_gamma0) + S * sin(_gamma0)) / T;
		auto const v = _a * log((1 - U) / (1 + U)) / (2 * _b);

		//Oblique Mercator
		auto u = (_a * atan((S * cos(_gamma0) + V * sin(_gamma0)) / cos(_b * (lambda - _longitude0))) / _b);

		if (fabs(_initialLineAzimuth - M_PI / 2) < 1E-10)
		{
			if (lambda == _longitude0)
				u = 0.0;
			else
				u = u - (fabs(_uc) * SIGN(_projectionCenterLatitude) * SIGN(_projectionCenterLongitude - lambda));
		}
		else
		{
			u = u - (fabs(_uc) * SIGN(_projectionCenterLatitude));
		}

		x = v * cos(_rectifiedGridAngle) + u * sin(_rectifiedGridAngle) + _eastingAtProjectionCenter;
		y = u * cos(_rectifiedGridAngle) - v * sin(_rectifiedGridAngle) + _northingAtProjectionCenter;
	}

	auto ObliqueMercator::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const vPrime = (x - _eastingAtProjectionCenter) * cos(_rectifiedGridAngle) - (y - _northingAtProjectionCenter) * sin(_rectifiedGridAngle);
		auto const uPrime = (y - _northingAtProjectionCenter) * cos(_rectifiedGridAngle) + (x - _eastingAtProjectionCenter) * sin(_rectifiedGridAngle) + (fabs(_uc) * SIGN(_projectionCenterLatitude));

		auto const QPrime = pow(M_E, (-_b * vPrime / _a));
		auto const SPrime = (QPrime - 1 / QPrime) / 2;
		auto const TPrime = (QPrime + 1 / QPrime) / 2;
		auto const VPrime = sin(_b * uPrime / _a);
		auto const UPrime = (VPrime * cos(_gamma0) + SPrime * sin(_gamma0)) / TPrime;
		auto const tPrime = pow(_h / sqrt((1 + UPrime) / (1 - UPrime)), 1 / _b);
		auto const X = M_PI / 2 - 2 * atan(tPrime);

		phi = X + sin(2 * X) * (pow(_e, 2) / 2 + 5 * pow(_e, 4) / 24 + pow(_e, 6) / 12 + 13 * pow(_e, 8) / 360)
		+ sin(4 * X) * (7 * pow(_e, 4) / 48 + 29 * pow(_e, 6) / 240 + 811 * pow(_e, 8) / 11520)
		+ sin(6 * X) * (7 * pow(_e, 6) / 120 + 81 * pow(_e, 8) / 1120)
		+ sin(8 * X) * (4279 * pow(_e, 8) / 161280);

		// It took me a whole afternoon (a Sunday one) to pass test 5105_part_2 because of the atan being an atan2 (I found out by reading the book 
		// Map Projections, A Working Manual, because in document G7-2 they consider it to be an atan.
		lambda = _longitude0 - atan2((SPrime * cos(_gamma0) - VPrime * sin(_gamma0)), cos(_b * uPrime / _a)) / _b;
	}
#pragma endregion
#pragma region IMathTransform members
	auto ObliqueMercator::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<ObliqueMercator>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
