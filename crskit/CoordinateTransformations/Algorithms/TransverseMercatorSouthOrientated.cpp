#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "TransverseMercatorSouthOrientated.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	TransverseMercatorSouthOrientated::TransverseMercatorSouthOrientated(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _latitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_origin").GetValue()))}
		, _longitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("central_meridian").GetValue()))}
		, _scaleFactorAtNaturalOrigin{std::get<double>(parameters->GetParameter("scale_factor").GetValue())}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		_a = _semiMajorAxis;
		auto const f = (_semiMajorAxis - _semiMinorAxis) / _semiMajorAxis;
		_e = sqrt(2 * f - f * f);
		_e2 = pow(_e, 2.0);
		_e4 = pow(_e, 4.0);
		_e6 = pow(_e, 6.0);
		//e8 = pow(e, 8.0);

		_m0 = _a * ((1.0 - _e2 / 4 - 3.0 * _e4 / 64 - 5 * _e6 / 256) * _latitudeOfNaturalOrigin
			- (3 * _e2 / 8 + 3 * _e4 / 32 + 45 * _e6 / 1024) * sin(2 * _latitudeOfNaturalOrigin)
			+ (15 * _e4 / 256 + 45 * _e6 / 1024) * sin(4 * _latitudeOfNaturalOrigin)
			- (35 * _e6 / 3072) * sin(6 * _latitudeOfNaturalOrigin));

		// At http://www.apc.univ-paris7.fr/~revenu/Auger/Geodesie/LatLong-UTMConversion.cc it comes with more precision, but there is hardly any difference
		//M0 = a*((1.0 - e2/4 - 3.0*e4/64 - 5*e6/256 -175*e8/16384 )*latitudeOfNaturalOrigin 
		//	- (3*e2/8 + 3*e4/32 + 45*e6/1024 + 105*e8/4096)*sin(2*latitudeOfNaturalOrigin) 
		//	+ (15*e4/256 + 45*e6/1024 + 525*e8/16384)*sin(4*latitudeOfNaturalOrigin)
		//	- (35*e6/3072 + 175*e8/12288)*sin(6*latitudeOfNaturalOrigin))
		//	+ 315*e8/131072*sin(8*latitudeOfNaturalOrigin);

		_e1 = (1 - sqrt(1 - _e2)) / (1 + sqrt(1 - _e2));
		_e_2 = _e2 / (1 - _e2);
	}

#pragma region MapProjectionBase members
	auto TransverseMercatorSouthOrientated::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const T = pow(tan(phi), 2.0);
		auto const C = _e2 * pow(cos(phi), 2.0) / (1.0 - _e2);
		auto const A = (lambda - _longitudeOfNaturalOrigin) * cos(phi);
		auto const v = _a / sqrt(1.0 - _e2 * pow(sin(phi), 2.0));

		auto const M = _a * ((1.0 - _e2 / 4 - 3.0 * _e4 / 64 - 5 * _e6 / 256) * phi
			- (3 * _e2 / 8 + 3 * _e4 / 32 + 45 * _e6 / 1024) * sin(2 * phi)
			+ (15 * _e4 / 256 + 45 * _e6 / 1024) * sin(4 * phi)
			- (35 * _e6 / 3072) * sin(6 * phi));

		// At http://www.apc.univ-paris7.fr/~revenu/Auger/Geodesie/LatLong-UTMConversion.cc it comes with more precision, but there is hardly any difference
		//M = a*((1.0 - e2/4 - 3.0*e4/64 - 5*e6/256 -175*e8/16384 )*phi
		//	- (3*e2/8 + 3*e4/32 + 45*e6/1024 + 105*e8/4096)*sin(2*phi) 
		//	+ (15*e4/256 + 45*e6/1024 + 525*e8/16384)*sin(4*phi)
		//	- (35*e6/3072 + 175*e8/12288)*sin(6*phi))
		//	+ 315*e8/131072*sin(8*phi);

		x = _falseEasting - _scaleFactorAtNaturalOrigin * v * (A + (1 - T + C) * A * A * A / 6 + (5 - 18 * T + T * T + 72 * C - 58 * _e_2) * A * A * A * A * A / 120);
		y = _falseNorthing - _scaleFactorAtNaturalOrigin * (M - _m0 + v * tan(phi) * (A * A / 2.0 + (5 - T + 9 * C + 4 * C * C) * A * A * A * A / 24 + (61 - 58 * T + T * T + 600 * C - 330 * _e_2) * A * A * A * A * A * A / 720));
	}

	auto TransverseMercatorSouthOrientated::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const M1 = _m0 - (y - _falseNorthing) / _scaleFactorAtNaturalOrigin;
		auto const u1 = M1 / (_a * (1 - _e2 / 4 - 3 * _e4 / 64 - 5 * _e6 / 256));
		auto const phi1 = u1 + (3 * _e1 / 2 - 27 * _e1 * _e1 * _e1 / 32) * sin(2 * u1)
		+ (21 * _e1 * _e1 / 16 - 55 * _e1 * _e1 * _e1 * _e1 / 32) * sin(4 * u1)
		+ (151 * _e1 * _e1 * _e1 / 96) * sin(6 * u1)
		+ (1097 * _e1 * _e1 * _e1 * _e1 / 512) * sin(8 * u1);

		auto const v1 = _a / sqrt(1 - _e2 * pow(sin(phi1), 2.0));
		auto const p1 = _a * (1 - _e2) / pow(1 - _e2 * pow(sin(phi1), 2.0), 1.5);
		auto const T1 = pow(tan(phi1), 2.0);
		auto const C1 = _e_2 * pow(cos(phi1), 2.0);
		auto const D = -(x - _falseEasting) / (v1 * _scaleFactorAtNaturalOrigin);

		phi = phi1 - (v1 * tan(phi1) / p1) * (D * D / 2 - (5 + 3 * T1 + 10 * C1 - 4 * C1 * C1 - 9 * _e_2) * D * D * D * D / 24
			+ (61 + 90 * T1 + 298 * C1 + 45 * T1 * T1 - 252 * _e_2 - 3 * C1 * C1) * D * D * D * D * D * D / 720);
		lambda = _longitudeOfNaturalOrigin +
		(D - (1 + 2 * T1 + C1) * D * D * D / 6 + (5 - 2 * C1 + 28 * T1 - 3 * C1 * C1 + 8 * _e_2 + 24 * T1 * T1) * D * D * D * D * D / 120) / cos(phi1);
	}
#pragma endregion
#pragma region IMathTransform members
	auto TransverseMercatorSouthOrientated::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<TransverseMercatorSouthOrientated>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
