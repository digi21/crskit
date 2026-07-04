#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "CassiniSoldner.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	CassiniSoldner::CassiniSoldner(std::shared_ptr<IProjection> const& parameters, bool const _inverse)
		: MapProjectionBase{parameters, _inverse}
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
	auto CassiniSoldner::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const M = _semiMajorAxis * ((1 - pow(_e, 2) / 4 - 3 * pow(_e, 4) / 64 - 5 * pow(_e, 6) / 256) * phi
			- (3 * pow(_e, 2) / 8 + 3 * pow(_e, 4) / 32 + 45 * pow(_e, 6) / 1024) * sin(2 * phi)
			+ (15 * pow(_e, 4) / 256 + 45 * pow(_e, 6) / 1024) * sin(4 * phi)
			- (35 * pow(_e, 6) / 3072) * sin(6 * phi));

		auto const v = _semiMajorAxis / sqrt(1 - pow(_e, 2) * pow(sin(phi), 2));
		auto const C = pow(_e, 2) * pow(cos(phi), 2) / (1 - pow(_e, 2));
		auto const T = pow(tan(phi), 2);
		auto const A = (lambda - _falseOriginLongitude) * cos(phi);
		auto const X = M - _m0 + v * tan(phi) * (pow(A, 2) / 2 + (5 - T + 6 * C) * pow(A, 4) / 24);

		x = _falseEasting + v * (A - T * pow(A, 3) / 6 - (8 - T + 8 * C) * T * pow(A, 5) / 120);
		y = _falseNorthing + X;
	}

	auto CassiniSoldner::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const M1 = _m0 + (y - _falseNorthing);

		auto const mu1 = M1 / (_semiMajorAxis * (1 - pow(_e, 2) / 4 - 3 * pow(_e, 4) / 64 - 5 * pow(_e, 6) / 256));
		auto const e1 = (1 - sqrt(1 - pow(_e, 2))) / (1 + sqrt(1 - pow(_e, 2)));
		auto const latitude1 = mu1 + (3 * e1 / 2 - 27 * pow(e1, 3) / 32) * sin(2 * mu1)
		+ (21 * pow(e1, 2) / 16 - 55 * pow(e1, 32)) * sin(4 * mu1)
		+ (151 * pow(e1, 3) / 96) * sin(6 * mu1)
		+ (1097 * pow(e1, 4) / 512) * sin(8 * mu1);

		auto const v1 = _semiMajorAxis / sqrt(1 - pow(_e, 2) * pow(sin(latitude1), 2));

		auto const D = (x - _falseEasting) / v1;
		auto const T1 = pow(tan(latitude1), 2);

		auto p1 = _semiMajorAxis * (1 - pow(_e, 2) / pow(1 - pow(_e, 2) * pow(sin(latitude1), 2), 1.5));

		lambda = _falseOriginLongitude + (D - T1 * pow(D, 3) / 3 + (1 + 3 * T1) * T1 * pow(D, 5) / 15) / cos(latitude1);
		phi = latitude1 - (v1 * tan(latitude1 / p1) * (pow(D, 2) / 2 - (1 + 3 * T1) * pow(D, 4) / 24));
	}
#pragma endregion

#pragma region IMathTransform members
	auto CassiniSoldner::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<CassiniSoldner>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
