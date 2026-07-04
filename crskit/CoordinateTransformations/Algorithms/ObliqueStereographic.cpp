#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "ObliqueStereographic.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	ObliqueStereographic::ObliqueStereographic(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _latitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_origin").GetValue()))}
		, _longitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("central_meridian").GetValue()))}
		, _scaleFactorAtNaturalOrigin{std::get<double>(parameters->GetParameter("scale_factor").GetValue())}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		ComputeConstants();
	}

	ObliqueStereographic::ObliqueStereographic(double semiMajorAxis, double semiMinorAxis, double latitudeOfOrigin, double centralMeridian, double scaleFactor, double falseEasting, double falseNorthing, bool inverse)
		: MapProjectionBase{nullptr, inverse}
		, _semiMinorAxis{semiMinorAxis}
		, _semiMajorAxis{semiMajorAxis}
		, _latitudeOfNaturalOrigin{DEGREES_TO_RADIANS(latitudeOfOrigin)}
		, _longitudeOfNaturalOrigin{DEGREES_TO_RADIANS(centralMeridian)}
		, _scaleFactorAtNaturalOrigin{scaleFactor}
		, _falseEasting{falseEasting}
		, _falseNorthing{falseNorthing}
	{
		ComputeConstants();
	}

	auto ObliqueStereographic::ComputeConstants() -> void
	{
		_a = _semiMajorAxis;
		_b = _semiMinorAxis;
		_f = (_a - _b) / _a;

		_e = sqrt(2 * _f - pow(_f, 2));
		_p0 = _a * (1 - pow(_e, 2)) / pow((1 - pow(_e, 2) * pow(sin(_latitudeOfNaturalOrigin), 2)), 1.5);
		_v0 = _a / sqrt((1 - pow(_e, 2) * pow(sin(_latitudeOfNaturalOrigin), 2)));
		_r = sqrt(_p0 * _v0);

		_n = sqrt(1 + ((pow(_e, 2) * pow(cos(_latitudeOfNaturalOrigin), 4)) / (1 - pow(_e, 2))));

		_s1 = (1 + sin(_latitudeOfNaturalOrigin)) / (1 - sin(_latitudeOfNaturalOrigin));
		_s2 = (1 - _e * sin(_latitudeOfNaturalOrigin)) / (1 + _e * sin(_latitudeOfNaturalOrigin));

		_w1 = pow(_s1 * pow(_s2, _e), _n);

		//the value of sin X0 has been substituted in the formula
		_c = (_n + sin(_latitudeOfNaturalOrigin)) * (1 - ((_w1 - 1) / (_w1 + 1))) / ((_n - sin(_latitudeOfNaturalOrigin)) * (1 + ((_w1 - 1) / (_w1 + 1))));

		_w2 = _c * _w1;

		_x0 = asin(((_w2 - 1) / (_w2 + 1)));
		//L is the uppercase lambda in G7-2
		_l0 = _longitudeOfNaturalOrigin;

		_g = 2 * _r * _scaleFactorAtNaturalOrigin * tan(M_PI / 4 - _x0 / 2);
		_h = 4 * _r * _scaleFactorAtNaturalOrigin * tan(_x0) + _g;
	}

#pragma region MapProjectionBase members
	auto ObliqueStereographic::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const sinPhi = sin(phi);

		auto const L = _n * (lambda - _l0) + _l0;
		auto const Sa = (1 + sinPhi) / (1 - sinPhi);
		auto const Sb = (1 - _e * sinPhi) / (1 + _e * sinPhi);
		auto const w = _c * pow(Sa * pow(Sb, _e), _n);

		auto const X = asin(((w - 1) / (w + 1)));

		auto const B = (1 + sin(X) * sin(_x0) + cos(X) * cos(_x0) * cos(L - _l0));

		x = _falseEasting + 2 * _r * _scaleFactorAtNaturalOrigin * cos(X) * sin(L - _l0) / B;
		y = _falseNorthing + 2 * _r * _scaleFactorAtNaturalOrigin * (sin(X) * cos(_x0) - cos(X) * sin(_x0) * cos(L - _l0)) / B;
	}

	auto ObliqueStereographic::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const i = atan((x - _falseEasting) / (_h + (y - _falseNorthing)));
		auto const j = atan((x - _falseEasting) / (_g - (y - _falseNorthing))) - i;

		auto const X = _x0 + 2 * atan(((y - _falseNorthing) - (x - _falseEasting) * tan(j / 2)) / (2 * _r * _scaleFactorAtNaturalOrigin));
		auto const L = j + 2 * i + _l0;

		lambda = (L - _l0) / _n + _l0;

		auto const isometricLatitude = 0.5 * log((1 + sin(X)) / (_c * (1 - sin(X)))) / _n;

		auto const latitude1 = 2 * atan(pow(M_E, isometricLatitude)) - M_PI / 2;
		auto latitudeOld = 0.0;
		phi = latitude1;
		int iteration = 0;

		while (fabs(latitudeOld - phi) > 1e-10 && ++iteration < 50) // cap: avoid an infinite loop on non-convergent / NaN input
		{
			latitudeOld = phi;
			auto const isometricLatitudeI = log((tan(phi / 2 + M_PI / 4)) * pow((1 - _e * sin(phi)) / (1 + _e * sin(phi)), _e / 2));
			phi = phi - (isometricLatitudeI - isometricLatitude) * cos(phi) * (1 - pow(_e, 2) * pow(sin(phi), 2)) / (1 - pow(_e, 2));
		}
	}
#pragma endregion
#pragma region IMathTransform members
	auto ObliqueStereographic::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<ObliqueStereographic>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
