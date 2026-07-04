#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "AmericanPolyconic.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	AmericanPolyconic::AmericanPolyconic(std::shared_ptr<IProjection> const& parameters, bool const inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _falseOriginLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_origin").GetValue()))}
		, _falseOriginLongitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("central_meridian").GetValue()))}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		_f = (_semiMajorAxis - _semiMinorAxis) / _semiMajorAxis;
		_e = sqrt(2 * _f - pow(_f, 2));

		_m0 = _semiMajorAxis * ((1 - pow(_e, 2) / 4 - 3 * pow(_e, 4) / 64 - 5 * pow(_e, 6) / 256) * _falseOriginLatitude
			- (3 * pow(_e, 2) / 8 + 3 * pow(_e, 4) / 32 + 45 * pow(_e, 6) / 1024) * sin(2 * _falseOriginLatitude)
			+ (15 * pow(_e, 4) / 256 + 45 * pow(_e, 6) / 1024) * sin(4 * _falseOriginLatitude)
			- (35 * pow(_e, 6) / 3072) * sin(6 * _falseOriginLatitude));
	}

#pragma region MapProjectionBase members
	auto AmericanPolyconic::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		if (fabs(phi) < 1e-10)
		{
			x = _falseEasting + _semiMajorAxis * (lambda - _falseOriginLongitude);
			y = _falseNorthing - _m0;
		}
		else
		{
			auto const L = (lambda - _falseOriginLongitude) * sin(phi);

			auto const M = _semiMajorAxis * ((1 - pow(_e, 2) / 4 - 3 * pow(_e, 4) / 64 - 5 * pow(_e, 6) / 256) * phi
				- (3 * pow(_e, 2) / 8 + 3 * pow(_e, 4) / 32 + 45 * pow(_e, 6) / 1024) * sin(2 * phi)
				+ (15 * pow(_e, 4) / 256 + 45 * pow(_e, 6) / 1024) * sin(4 * phi)
				- (35 * pow(_e, 6) / 3072) * sin(6 * phi));
			auto const v = _semiMajorAxis / sqrt(1 - pow(_e, 2) * pow(sin(phi), 2));

			x = _falseEasting + (v / tan(phi)) * sin(L);
			y = _falseNorthing + M + _m0 + (v / tan(phi)) * (1 - cos(L));
		}
	}

	auto AmericanPolyconic::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		y = y - _falseNorthing;
		x = x - _falseEasting;

		if (-_m0 == y)
		{
			phi = 0.0;
			lambda = _falseOriginLongitude + x / _semiMajorAxis;
		}
		else
		{
			auto const A = (_m0 + y) / _semiMajorAxis;
			auto const B = pow(x, 2) / pow(_semiMajorAxis, 2) + pow(A, 2);

			phi = A;

			auto phiOld = 0.0;
			auto C = 0.0;
			int iteration = 0;
			while (fabs(phiOld - phi) > 1e-10 && ++iteration < 50) // cap: avoid an infinite loop on non-convergent / NaN input
			{
				C = sqrt(1 - pow(_e, 2) * pow(sin(phi), 2)) * tan(phi);
				auto const Mn = _semiMajorAxis * ((1 - pow(_e, 2) / 4 - 3 * pow(_e, 4) / 64 - 5 * pow(_e, 6) / 256) * phi
					- (3 * pow(_e, 2) / 8 + 3 * pow(_e, 4) / 32 + 45 * pow(_e, 6) / 1024) * sin(2 * phi)
					+ (15 * pow(_e, 4) / 256 + 45 * pow(_e, 6) / 1024) * sin(4 * phi)
					- (35 * pow(_e, 6) / 3072) * sin(6 * phi));

				auto const MnPrime = 1 - pow(_e, 2) / 4 - 3 * pow(_e, 4) / 64 - 5 * pow(_e, 6) / 256
				- 2 * (3 * pow(_e, 2) / 8 + 3 * pow(_e, 4) / 32 + 45 * pow(_e, 6) / 1024) * cos(2 * phi)
				+ 4 * (15 * pow(_e, 4) / 256 + 45 * pow(_e, 6) / 1024) * cos(4 * phi)
				- 6 * (35 * pow(_e, 6) / 3072) * cos(6 * phi);

				auto const Ma = Mn / _semiMajorAxis;

				phiOld = phi;
				phi = phi - (A * (C * Ma + 1) - Ma - 0.5 * (pow(Ma, 2) + B) * C) /
				(pow(_e, 2) * sin(2 * phi) * (pow(Ma, 2) + B - 2 * A * Ma) /
					4 * C + (A - Ma) * (C * MnPrime - 2 / sin(2 * phi)) - MnPrime);
			}
			lambda = asin(clampToUnit(x * C / _semiMajorAxis)) / sin(phi) + _falseOriginLongitude;
		}
	}
#pragma endregion

#pragma region IMathTransform members
	auto AmericanPolyconic::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<AmericanPolyconic>(_parameters, !_inverse);

		return _inverseTransformation;
	}
#pragma endregion
}
