#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "Mercator2Sp.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	Mercator2Sp::Mercator2Sp(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _firstStandardParallelLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("standard_parallel_1").GetValue()))}
		, _longitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("central_meridian").GetValue()))}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		_latitude1 = _firstStandardParallelLatitude;
		_longitude0 = _longitudeOfNaturalOrigin;

		auto const a = _semiMajorAxis;
		auto const b = _semiMinorAxis;
		auto const f = (a - b) / a;
		_e = sqrt(2 * f - f * f);
		_e2 = _e * _e;
		_e4 = _e2 * _e2;
		_e6 = _e4 * _e2;
		_e8 = _e6 * _e2;
		_k0 = cos(_latitude1) / sqrt(1 - _e2 * pow(sin(fabs(_latitude1)), 2));
	}

#pragma region MapProjectionBase members
	auto Mercator2Sp::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const sinPhi = sin(phi);

		x = _falseEasting + _semiMajorAxis * _k0 * (lambda - _longitude0);
		y = _falseNorthing + _semiMajorAxis * _k0 * log(tan(M_PI / 4 + phi / 2) * pow((1 - _e * sinPhi) / (1 + _e * sinPhi), (_e / 2)));
	}

	auto Mercator2Sp::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const t = pow(M_E, (_falseNorthing - y) / (_semiMajorAxis * _k0));
		auto const X = M_PI / 2 - 2 * atan(t);

		phi = X + (_e2 / 2 + 5 * _e4 / 24 + _e6 / 12 + 13 * _e8 / 360) * sin(2 * X)
		+ (7 * _e4 / 48 + 29 * _e6 / 240 + 811 * _e8 / 11520) * sin(4 * X)
		+ (7 * _e6 / 120 + 81 * _e8 / 1120) * sin(6 * X)
		+ (4279 * _e8 / 161280) * sin(8 * X);

		lambda = ((x - _falseEasting) / (_semiMajorAxis * _k0)) + _longitude0;
	}
#pragma endregion
#pragma region IMathTransform members
	auto Mercator2Sp::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<Mercator2Sp>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
