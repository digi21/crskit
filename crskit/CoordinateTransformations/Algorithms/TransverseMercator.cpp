#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "TransverseMercator.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	TransverseMercator::TransverseMercator(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _latitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_origin").GetValue()))}
		, _longitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("central_meridian").GetValue()))}
		, _scaleFactorAtNaturalOrigin{std::get<double>(parameters->GetParameter("scale_factor").GetValue())}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
#ifdef _FORMULAS_JHS
		_a = _semiMajorAxis;
		_b = _semiMinorAxis;
		_lambdaO = _longitudeOfNaturalOrigin;
		_phiO = _latitudeOfNaturalOrigin;
		_fe = _falseEasting;
		_fn = _falseNorthing;
		_kO = _scaleFactorAtNaturalOrigin;

		_f = (_a - _b) / _a;

		_e = sqrt(2 * _f - _f * _f);
		_n = _f / (2 - _f);
		_n2 = _n * _n;
		_n3 = _n2 * _n;
		_n4 = _n3 * _n;

		_B = (_a / (1 + _n)) * (1 + _n2 / 4.0 + _n4 / 64.0);

		_h1 = _n / 2 - (2.0 / 3.0) * _n2 + (5.0 / 16.0) * _n3 + (41.0 / 180.0) * _n4;
		_h2 = (13.0 / 48.0) * _n2 - (3.0 / 5.0) * _n3 + (557.0 / 1440.0) * _n4;
		_h3 = (61.0 / 240.0) * _n3 - (103.0 / 140.0) * _n4;
		_h4 = (49561.0 / 161280.0) * _n4;

		_h1P = _n / 2 - (2.0 / 3.0) * _n2 + (37.0 / 96.0) * _n3 - (1.0 / 360.0) * _n4;
		_h2P = (1.0 / 48.0) * _n2 + (1.0 / 15.0) * _n3 - (437.0 / 1440.0) * _n4;
		_h3P = (17.0 / 480.0) * _n3 - (37.0 / 840.0) * _n4;
		_h4P = (4397.0 / 161280.0) * _n4;

		// Then the meridional arc distance from equator to the projection origin (SO) is computed from:
		if (_phiO == 0)
		{
			_so = 0;
		}
		else
		{
			auto const Q_0 = asinh(tan(_phiO)) - _e * atanh(_e * sin(_phiO));
			auto const betaO = atan(sinh(Q_0));
			auto const xiO0 = asin(sin(betaO));
			// Note: The previous two steps are taken from the generic calculation flow given below for;
			// latitude phi, but here for phiO may be simplified to xiO0 = betaO = atan(sinh QO).;
			auto const xiO1 = _h1 * sin(2 * xiO0);
			auto const xiO2 = _h2 * sin(4 * xiO0);
			auto const xiO3 = _h3 * sin(6 * xiO0);
			auto const xiO4 = _h4 * sin(8 * xiO0);
			auto const xiO = xiO0 + xiO1 + xiO2 + xiO3 + xiO4;
			_so = _B * xiO;
		}

#else
						a = semiMajorAxis;
						double f = (semiMajorAxis - semiMinorAxis)/semiMajorAxis;
						e = sqrt(2*f-f*f);
						e2 = pow(e, 2.0);
						e4 = pow(e, 4.0);
						e6 = pow(e, 6.0);
						//e8 = pow(e, 8.0);

						M0 = a*((1.0 - e2/4 - 3.0*e4/64 - 5*e6/256 )*latitudeOfNaturalOrigin 
							- (3*e2/8 + 3*e4/32 + 45*e6/1024)*sin(2*latitudeOfNaturalOrigin)
							+ (15*e4/256 + 45*e6/1024)*sin(4*latitudeOfNaturalOrigin) 
							- (35*e6/3072)*sin(6*latitudeOfNaturalOrigin));

						// At http://www.apc.univ-paris7.fr/~revenu/Auger/Geodesie/LatLong-TransverseMercatorConversion.cc it comes with more precision, but there is hardly any difference
						//M0 = a*((1.0 - e2/4 - 3.0*e4/64 - 5*e6/256 -175*e8/16384 )*latitudeOfNaturalOrigin 
						//	- (3*e2/8 + 3*e4/32 + 45*e6/1024 + 105*e8/4096)*sin(2*latitudeOfNaturalOrigin) 
						//	+ (15*e4/256 + 45*e6/1024 + 525*e8/16384)*sin(4*latitudeOfNaturalOrigin)
						//	- (35*e6/3072 + 175*e8/12288)*sin(6*latitudeOfNaturalOrigin))
						//	+ 315*e8/131072*sin(8*latitudeOfNaturalOrigin);

						e1 = (1 - sqrt(1-e2))/(1 + sqrt(1-e2));
						e_2 = e2/(1-e2);
#endif
	}

#pragma region MapProjectionBase members
	auto TransverseMercator::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
#ifdef _FORMULAS_JHS
		auto const Q = asinh(tan(phi)) - (_e * atanh(_e * sin(phi)));
		auto const beta = atan(sinh(Q));

		auto const eta0 = atanh(cos(beta) * sin(lambda - _lambdaO));
		auto const xi0 = asin(clampToUnit(sin(beta) * cosh(eta0)));

		auto const xi1 = _h1 * sin(2 * xi0) * cosh(2 * eta0);
		auto const xi2 = _h2 * sin(4 * xi0) * cosh(4 * eta0);
		auto const xi3 = _h3 * sin(6 * xi0) * cosh(6 * eta0);
		auto const xi4 = _h4 * sin(8 * xi0) * cosh(8 * eta0);
		auto const xi = xi0 + xi1 + xi2 + xi3 + xi4;

		auto const eta1 = _h1 * cos(2 * xi0) * sinh(2 * eta0);
		auto const eta2 = _h2 * cos(4 * xi0) * sinh(4 * eta0);
		auto const eta3 = _h3 * cos(6 * xi0) * sinh(6 * eta0);
		auto const eta4 = _h4 * cos(8 * xi0) * sinh(8 * eta0);
		auto const eta = eta0 + eta1 + eta2 + eta3 + eta4;

		x = _fe + _kO * _B * eta;
		y = _fn + _kO * (_B * xi - _so);
#else
						double T = pow(tan(lambda), 2.0);
						double C = e2 * pow(cos(lambda), 2.0) / (1.0 - e2);
						double A = (phi - longitudeOfNaturalOrigin) * cos(lambda);
						double v = a/sqrt(1.0 - e2*pow(sin(lambda), 2.0));

						double M = a*((1.0 - e2/4 - 3.0*e4/64 - 5*e6/256 )*lambda 
							- (3*e2/8 + 3*e4/32 + 45*e6/1024)*sin(2*lambda) 
							+ (15*e4/256 + 45*e6/1024)*sin(4*lambda)
							- (35*e6/3072)*sin(6*lambda));

						// At http://www.apc.univ-paris7.fr/~revenu/Auger/Geodesie/LatLong-TransverseMercatorConversion.cc it comes with more precision, but there is hardly any difference
						//M = a*((1.0 - e2/4 - 3.0*e4/64 - 5*e6/256 -175*e8/16384 )*lambda
						//	- (3*e2/8 + 3*e4/32 + 45*e6/1024 + 105*e8/4096)*sin(2*lambda) 
						//	+ (15*e4/256 + 45*e6/1024 + 525*e8/16384)*sin(4*lambda)
						//	- (35*e6/3072 + 175*e8/12288)*sin(6*lambda))
						//	+ 315*e8/131072*sin(8*lambda);

						x = falseEasting + scaleFactorAtNaturalOrigin*v*(A + (1 - T + C)*A*A*A/6 + (5 - 18*T + T*T + 72*C - 58*e_2)*A*A*A*A*A/120);
						y = falseNorthing + scaleFactorAtNaturalOrigin*(M - M0 + v*tan(lambda)*(A*A/2.0 + (5 - T + 9*C + 4*C*C)*A*A*A*A/24 + (61 - 58*T + T*T + 600*C - 330*e_2)*A*A*A*A*A*A/720));
#endif
	}

	auto TransverseMercator::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
#ifdef _FORMULAS_JHS
		//	For the reverse formulas to convert Easting and Northing projected coordinates to latitude and longitude first
		//	calculate constants of the projection where n is as for the forward conversion, as are B and SO:
		auto const eta = (x - _fe) / (_B * _kO);
		auto const xi = ((y - _fn) + _kO * _so) / (_B * _kO);

		auto const xi1 = _h1P * sin(2 * xi) * cosh(2 * eta);
		auto const xi2 = _h2P * sin(4 * xi) * cosh(4 * eta);
		auto const xi3 = _h3P * sin(6 * xi) * cosh(6 * eta);
		auto const xi4 = _h4P * sin(8 * xi) * cosh(8 * eta);
		auto const xi0 = xi - (xi1 + xi2 + xi3 + xi4);

		auto const eta1 = _h1P * cos(2 * xi) * sinh(2 * eta);
		auto const eta2 = _h2P * cos(4 * xi) * sinh(4 * eta);
		auto const eta3 = _h3P * cos(6 * xi) * sinh(6 * eta);
		auto const eta4 = _h4P * cos(8 * xi) * sinh(8 * eta);
		auto const eta0 = eta - (eta1 + eta2 + eta3 + eta4);
		auto const beta = asin(clampToUnit(sin(xi0) / cosh(eta0)));

		auto const Q = asinh(tan(beta));

		auto Q2 = Q + _e * atanh(_e * tanh(Q));

		double dif;
		int iteration = 0;

		do
		{
			auto const Q2tmp = Q + _e * atanh(_e * tanh(Q2));
			dif = fabs(Q2 - Q2tmp);
			Q2 = Q2tmp;
		}
		while (dif > 1E-8 && ++iteration < 50); // cap: avoid an infinite loop on non-convergent / NaN input

		phi = atan(sinh(Q2));
		lambda = (_lambdaO + asin(clampToUnit(tanh(eta0) / cos(beta))));
#else
						auto const M1 = M0 + (y - falseNorthing)/scaleFactorAtNaturalOrigin;
						auto const u1 = M1/(a*(1 - e2/4 - 3*e4/64 - 5*e6/256));
						double phi1 = u1 + (3*e1/2 - 27*e1*e1*e1/32)*sin(2*u1) 
							+ (21*e1*e1/16 - 55*e1*e1*e1*e1/32)*sin(4*u1)
							+ (151*e1*e1*e1/96)*sin(6*u1)
							+ (1097*e1*e1*e1*e1/512)*sin(8*u1);

						double v1 = a/sqrt(1 - e2*pow(sin(phi1), 2.0));
						double p1 = a*(1 - e2)/pow(1 - e2*pow(sin(phi1), 2.0), 1.5);
						double T1 = pow(tan(phi1), 2.0);
						double C1 = e_2*pow(cos(phi1), 2.0);
						double D = (x - falseEasting)/(v1*scaleFactorAtNaturalOrigin);

						phi = (phi1 - (v1*tan(phi1)/p1)*(D*D/2 - (5 + 3*T1 + 10*C1 - 4*C1*C1 - 9*e_2)*D*D*D*D/24
							+ (61 + 90*T1 + 298*C1 + 45*T1*T1 - 252*e_2 - 3*C1*C1)*D*D*D*D*D*D/720));
						lambda = (longitudeOfNaturalOrigin + 
							(D - (1 + 2*T1 + C1)*D*D*D/6 + (5 - 2*C1 + 28*T1 - 3*C1*C1 + 8*e_2 + 24*T1*T1)*D*D*D*D*D/120)/cos(phi1));
#endif
	}
#pragma endregion
#pragma region IMathTransform members
	auto TransverseMercator::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<TransverseMercator>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
