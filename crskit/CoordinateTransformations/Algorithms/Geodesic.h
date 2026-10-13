#pragma once

#include <cmath>

namespace CrsKit::CoordinateTransformations::Algorithms
{
	// Direct and inverse geodesic problems on an ellipsoid (Vincenty 1975). Angles in radians, distance in the unit of a.
	// Within 0.1 mm of Karney's algorithms except near the antipode, where the inverse iteration may not converge.
	class Geodesic
	{
		double _a;
		double _f;
		double _b;

		auto Coefficients(double cos2Alpha, double& bigA, double& bigB) const -> void
		{
			auto const u2 = cos2Alpha * (_a * _a - _b * _b) / (_b * _b);
			bigA = 1 + u2 / 16384 * (4096 + u2 * (-768 + u2 * (320 - 175 * u2)));
			bigB = u2 / 1024 * (256 + u2 * (-128 + u2 * (74 - 47 * u2)));
		}

		static auto DeltaSigma(double bigB, double sinSigma, double cosSigma, double cos2SigmaM) -> double
		{
			auto const c2 = cos2SigmaM * cos2SigmaM;
			return bigB * sinSigma * (cos2SigmaM + bigB / 4 * (cosSigma * (-1 + 2 * c2)
				- bigB / 6 * cos2SigmaM * (-3 + 4 * sinSigma * sinSigma) * (-3 + 4 * c2)));
		}

	public:
		Geodesic(double a, double f) : _a{a}, _f{f}, _b{a * (1 - f)} {}

		// Distance s and forward azimuth from (lat1, lon1) to (lat2, lon2).
		auto Inverse(double lat1, double lon1, double lat2, double lon2, double& s, double& azimuth) const -> void
		{
			auto const l = std::remainder(lon2 - lon1, 2 * 3.141592653589793);
			auto const u1 = std::atan((1 - _f) * std::tan(lat1));
			auto const u2 = std::atan((1 - _f) * std::tan(lat2));
			auto const sinU1 = std::sin(u1), cosU1 = std::cos(u1), sinU2 = std::sin(u2), cosU2 = std::cos(u2);

			auto lambda = l;
			double sinSigma{}, cosSigma{}, sigma{}, cos2Alpha{}, cos2SigmaM{};
			for (auto i = 0; i < 200; ++i)
			{
				auto const sinLambda = std::sin(lambda), cosLambda = std::cos(lambda);
				sinSigma = std::hypot(cosU2 * sinLambda, cosU1 * sinU2 - sinU1 * cosU2 * cosLambda);
				if (0.0 == sinSigma)
				{
					s = 0.0;
					azimuth = 0.0;
					return;
				}
				cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
				sigma = std::atan2(sinSigma, cosSigma);
				auto const sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
				cos2Alpha = 1 - sinAlpha * sinAlpha;
				cos2SigmaM = cos2Alpha != 0.0 ? cosSigma - 2 * sinU1 * sinU2 / cos2Alpha : 0.0;
				auto const c = _f / 16 * cos2Alpha * (4 + _f * (4 - 3 * cos2Alpha));
				auto const previous = lambda;
				lambda = l + (1 - c) * _f * sinAlpha * (sigma + c * sinSigma * (cos2SigmaM + c * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
				if (std::fabs(lambda - previous) < 1e-14)
					break;
			}

			double bigA, bigB;
			Coefficients(cos2Alpha, bigA, bigB);
			s = _b * bigA * (sigma - DeltaSigma(bigB, sinSigma, cosSigma, cos2SigmaM));
			azimuth = std::atan2(cosU2 * std::sin(lambda), cosU1 * sinU2 - sinU1 * cosU2 * std::cos(lambda));
		}

		// The point at distance s and forward azimuth from (lat1, lon1).
		auto Direct(double lat1, double lon1, double azimuth, double s, double& lat2, double& lon2) const -> void
		{
			auto const u1 = std::atan((1 - _f) * std::tan(lat1));
			auto const sinU1 = std::sin(u1), cosU1 = std::cos(u1);
			auto const sinAz = std::sin(azimuth), cosAz = std::cos(azimuth);
			auto const sigma1 = std::atan2(std::tan(u1), cosAz);
			auto const sinAlpha = cosU1 * sinAz;
			auto const cos2Alpha = 1 - sinAlpha * sinAlpha;

			double bigA, bigB;
			Coefficients(cos2Alpha, bigA, bigB);
			auto sigma = s / (_b * bigA);
			double cos2SigmaM{}, sinSigma{}, cosSigma{};
			for (auto i = 0; i < 200; ++i)
			{
				cos2SigmaM = std::cos(2 * sigma1 + sigma);
				sinSigma = std::sin(sigma);
				cosSigma = std::cos(sigma);
				auto const next = s / (_b * bigA) + DeltaSigma(bigB, sinSigma, cosSigma, cos2SigmaM);
				auto const delta = std::fabs(next - sigma);
				sigma = next;
				if (delta < 1e-14)
					break;
			}
			cos2SigmaM = std::cos(2 * sigma1 + sigma);
			sinSigma = std::sin(sigma);
			cosSigma = std::cos(sigma);

			auto const tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAz;
			lat2 = std::atan2(sinU1 * cosSigma + cosU1 * sinSigma * cosAz, (1 - _f) * std::hypot(sinAlpha, tmp));
			auto const lambda = std::atan2(sinSigma * sinAz, cosU1 * cosSigma - sinU1 * sinSigma * cosAz);
			auto const c = _f / 16 * cos2Alpha * (4 + _f * (4 - 3 * cos2Alpha));
			lon2 = lon1 + lambda - (1 - c) * _f * sinAlpha * (sigma + c * sinSigma * (cos2SigmaM + c * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
		}
	};
}