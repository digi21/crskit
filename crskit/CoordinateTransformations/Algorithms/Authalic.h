#pragma once

#include <cmath>

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	// Authalic (equal-area) latitude of an ellipsoid with eccentricity e (GN7-2, Snyder 3-11 to 3-16). e may be 0.
	class Authalic
	{
		double _e;
		double _qP;

	public:
		explicit Authalic(double e) : _e{e}, _qP{Q(1.5707963267948966)} {}

		// q of GN7-2; 2 sin(phi) on a sphere.
		[[nodiscard]] auto Q(double phi) const -> double
		{
			auto const s = std::sin(phi);
			if (_e < 1e-12)
				return 2 * s;
			auto const e2 = _e * _e;
			return (1 - e2) * (s / (1 - e2 * s * s) - std::log((1 - _e * s) / (1 + _e * s)) / (2 * _e));
		}

		[[nodiscard]] auto QP() const -> double { return _qP; }

		// Radius of the sphere with the area of the ellipsoid: Rq = a (qP / 2)^0.5.
		[[nodiscard]] auto Radius(double a) const -> double { return a * std::sqrt(_qP / 2); }

		[[nodiscard]] auto Beta(double phi) const -> double
		{
			auto const ratio = Q(phi) / _qP;
			return std::asin(ratio < -1 ? -1 : ratio > 1 ? 1 : ratio);
		}

		// Geodetic latitude of the authalic latitude beta: the GN7-2 series refined with Newton on q.
		[[nodiscard]] auto Phi(double beta) const -> double
		{
			if (_e < 1e-12)
				return beta;
			auto const e2 = _e * _e;
			auto const e4 = e2 * e2;
			auto const e6 = e4 * e2;
			auto phi = beta + (e2 / 3 + 31 * e4 / 180 + 517 * e6 / 5040) * std::sin(2 * beta)
				+ (23 * e4 / 360 + 251 * e6 / 3780) * std::sin(4 * beta)
				+ (761 * e6 / 45360) * std::sin(6 * beta);
			auto const q = _qP * std::sin(beta);
			for (auto i = 0; i < 4 && std::fabs(std::cos(phi)) > 1e-12; ++i)
			{
				auto const s = std::sin(phi);
				auto const w = 1 - e2 * s * s;
				auto const delta = w * w / (2 * std::cos(phi)) * (q / (1 - e2) - s / w + std::log((1 - _e * s) / (1 + _e * s)) / (2 * _e));
				phi += delta;
				if (std::fabs(delta) < 1e-15)
					break;
			}
			return phi;
		}
	};
}