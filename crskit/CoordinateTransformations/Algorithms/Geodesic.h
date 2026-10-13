#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace CrsKit::CoordinateTransformations::Algorithms
{
	// Direct and inverse geodesic problems on an ellipsoid (Vincenty 1975). Angles in radians, distance in the unit of a.
	// Within 0.1 mm of Karney's algorithms. Near the antipode, where Vincenty's inverse iteration does not converge, the
	// inverse takes the shortest of the geodesics found by a scan of the initial azimuth.
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
			auto converged = false;
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
				{
					converged = true;
					break;
				}
			}

			if (!converged || std::fabs(lambda) > 3.141592653589793)
			{
				InverseByScan(lon1, lon2, u1, u2, s, azimuth);
				return;
			}

			double bigA, bigB;
			Coefficients(cos2Alpha, bigA, bigB);
			s = _b * bigA * (sigma - DeltaSigma(bigB, sinSigma, cosSigma, cos2SigmaM));
			azimuth = std::atan2(cosU2 * std::sin(lambda), cosU1 * sinU2 - sinU1 * cosU2 * std::cos(lambda));
		}

	private:
		// Every geodesic from (lat1, lon1) with azimuth alpha reaches the reduced latitude u2 at two arc lengths sigma on the
		// auxiliary sphere, in closed form; the longitude it has there, minus lon2, is a function of alpha whose roots are
		// the geodesics to the point. Near the antipode there are several: they are bracketed on a 720-step scan of alpha,
		// refined by bisection, and the shortest is kept.
		auto InverseByScan(double lon1, double lon2, double u1, double u2, double& s, double& azimuth) const -> void
		{
			constexpr double pi = 3.141592653589793;
			auto const dl = std::remainder(lon2 - lon1, 2 * pi);
			auto const sinU1 = std::sin(u1), cosU1 = std::cos(u1), sinU2 = std::sin(u2);

			// Near opposite poles, with the second point on the meridian opposite the first one, the scan cancels in omega;
			// the geodesic is the meridian over the nearer pole.
			if (_a * std::cos(u2) * std::fabs(std::sin(dl)) < 1e-6 && std::fabs(dl) > pi / 2 && std::max(cosU1, std::cos(u2)) < 1e-6)
			{
				auto const meridian = [&](double alpha, double sigma)
				{
					double bigAcoef, bigBcoef;
					Coefficients(1.0, bigAcoef, bigBcoef);
					auto const cos2SigmaM = std::cos(2 * std::atan2(std::tan(u1), std::cos(alpha)) + sigma);
					return _b * bigAcoef * (sigma - DeltaSigma(bigBcoef, std::sin(sigma), std::cos(sigma), cos2SigmaM));
				};
				auto const north = meridian(0.0, pi - u1 - u2), south = meridian(pi, pi + u1 + u2);
				s = std::min(north, south);
				azimuth = north <= south ? 0.0 : pi;
				return;
			}

			// Longitude mismatch and distance of the geodesic with azimuth alpha at its branch-th crossing of u2.
			auto const evaluate = [&](double alpha, int branch, double& mismatch, double& distance) -> bool
			{
				auto const bigA = sinU1, bigB = cosU1 * std::cos(alpha);
				auto const r = std::hypot(bigA, bigB);
				if (r < std::fabs(sinU2))
					return false;
				auto const phase = std::atan2(bigB, bigA);
				auto sigma = phase + (0 == branch ? 1 : -1) * std::acos(std::clamp(sinU2 / r, -1.0, 1.0));
				sigma = sigma - 2 * pi * std::floor(sigma / (2 * pi));	// 0 <= sigma < 2 pi
				auto const sinAlpha0 = cosU1 * std::sin(alpha);
				auto const cos2Alpha = 1 - sinAlpha0 * sinAlpha0;
				auto const sigma1 = std::atan2(std::tan(u1), std::cos(alpha));
				auto const cos2SigmaM = std::cos(2 * sigma1 + sigma);
				auto const omega = std::atan2(std::sin(sigma) * std::sin(alpha), cosU1 * std::cos(sigma) - sinU1 * std::sin(sigma) * std::cos(alpha));
				auto const c = _f / 16 * cos2Alpha * (4 + _f * (4 - 3 * cos2Alpha));
				auto const l = omega - (1 - c) * _f * sinAlpha0 * (sigma + c * std::sin(sigma) * (cos2SigmaM + c * std::cos(sigma) * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
				mismatch = std::remainder(l - dl, 2 * pi);
				double bigAcoef, bigBcoef;
				Coefficients(cos2Alpha, bigAcoef, bigBcoef);
				distance = _b * bigAcoef * (sigma - DeltaSigma(bigBcoef, std::sin(sigma), std::cos(sigma), cos2SigmaM));
				return true;
			};

			constexpr int steps = 720;
			auto best = std::numeric_limits<double>::infinity();
			for (auto branch = 0; branch < 2; ++branch)
			{
				double previousMismatch{};
				auto previousValid = false;
				for (auto i = 0; i <= steps; ++i)
				{
					auto const alpha = -pi + 2 * pi * i / steps;
					double mismatch, distance;
					auto const valid = evaluate(alpha, branch, mismatch, distance);
					if (valid && previousValid && (mismatch == 0.0 || (mismatch > 0) != (previousMismatch > 0)) && std::fabs(mismatch - previousMismatch) < pi)
					{
						auto lo = alpha - 2 * pi / steps, hi = alpha;
						auto loMismatch = previousMismatch;
						for (auto k = 0; k < 60; ++k)
						{
							auto const mid = (lo + hi) / 2;
							double midMismatch, midDistance;
							if (!evaluate(mid, branch, midMismatch, midDistance))
								break;
							if ((midMismatch > 0) == (loMismatch > 0))
							{
								lo = mid;
								loMismatch = midMismatch;
							}
							else
								hi = mid;
						}
						double rootMismatch, rootDistance;
						auto const root = (lo + hi) / 2;
						if (evaluate(root, branch, rootMismatch, rootDistance) && std::fabs(rootMismatch) < 1e-9 && rootDistance < best)
						{
							best = rootDistance;
							s = rootDistance;
							azimuth = root;
						}
					}
					previousValid = valid;
					previousMismatch = mismatch;
				}
			}
			if (std::isinf(best))
				s = azimuth = std::numeric_limits<double>::quiet_NaN();
		}
	public:

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