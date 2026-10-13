#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "PolarStereographic.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	namespace
	{
		auto Value(std::shared_ptr<IProjection> const& parameters, char const* name) -> double
		{
			return std::get<double>(parameters->GetParameter(name).GetValue());
		}

		auto HasParameter(std::shared_ptr<IProjection> const& parameters, char const* name) -> bool
		{
			return std::ranges::any_of(parameters->GetParameters(), [name](auto const& p) { return 0 == compareNoCase(p.GetName().c_str(), name); });
		}
	}

	PolarStereographic::PolarStereographic(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: PolarStereographic{parameters, inverse, Variant::AOrB}
	{
	}

	PolarStereographic::PolarStereographic(std::shared_ptr<IProjection> const& parameters, bool inverse, Variant variant)
		: MapProjectionBase{parameters, inverse}
	{
		auto const a = Value(parameters, "semi_major");
		auto const b = Value(parameters, "semi_minor");
		auto const f = (a - b) / a;
		_e = sqrt(2 * f - f * f);

		auto const latitude = DEGREES_TO_RADIANS(Value(parameters, "latitude_of_origin"));
		_south = latitude < 0;
		_longitudeOfOrigin = DEGREES_TO_RADIANS(Value(parameters, "central_meridian"));
		_falseEasting = Value(parameters, "false_easting");
		_falseNorthing = Value(parameters, "false_northing");

		auto const root = sqrt(pow(1 + _e, 1 + _e) * pow(1 - _e, 1 - _e));
		auto const isPole = fabs(fabs(latitude) - M_PI / 2) < 1e-10;
		if (variant == Variant::AOrB && isPole && HasParameter(parameters, "scale_factor"))
		{
			_k = 2 * a * Value(parameters, "scale_factor") / root;	// variant A
			return;
		}

		// Variants B and C: the standard parallel latF fixes the scale.
		auto const mF = cos(latitude) / sqrt(1 - _e * _e * sin(latitude) * sin(latitude));
		auto const tF = T(latitude);
		if (variant == Variant::C)
		{
			auto const rhoF = a * mF;
			_k = rhoF / tF;
			_falseNorthing += _south ? -rhoF : rhoF;
			return;
		}

		auto const k0 = mF * root / (2 * tF);
		_k = 2 * a * k0 / root;
	}

	auto PolarStereographic::T(double phi) const -> double
	{
		auto const ratio = pow((1 + _e * sin(phi)) / (1 - _e * sin(phi)), _e / 2);
		return _south ? tan(M_PI / 4 + phi / 2) / ratio : tan(M_PI / 4 - phi / 2) * ratio;
	}

	auto PolarStereographic::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const rho = _k * T(phi);
		auto const dLambda = LongitudeFromOrigin(lambda, _longitudeOfOrigin);
		x = _falseEasting + rho * sin(dLambda);
		y = _south ? _falseNorthing + rho * cos(dLambda) : _falseNorthing - rho * cos(dLambda);
	}

	auto PolarStereographic::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const dE = x - _falseEasting;
		auto const dN = y - _falseNorthing;
		auto const t = hypot(dE, dN) / _k;
		auto const chi = _south ? 2 * atan(t) - M_PI / 2 : M_PI / 2 - 2 * atan(t);

		auto const e2 = _e * _e;
		auto const e4 = e2 * e2;
		auto const e6 = e4 * e2;
		auto const e8 = e4 * e4;
		phi = chi + (e2 / 2 + 5 * e4 / 24 + e6 / 12 + 13 * e8 / 360) * sin(2 * chi)
			+ (7 * e4 / 48 + 29 * e6 / 240 + 811 * e8 / 11520) * sin(4 * chi)
			+ (7 * e6 / 120 + 81 * e8 / 1120) * sin(6 * chi)
			+ (4279 * e8 / 161280) * sin(8 * chi);

		lambda = (dE == 0.0 && dN == 0.0) ? _longitudeOfOrigin : _longitudeOfOrigin + atan2(dE, _south ? dN : -dN);
	}

	auto PolarStereographic::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<PolarStereographic>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	PolarStereographicVariantB::PolarStereographicVariantB(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: PolarStereographic{parameters, inverse, Variant::AOrB}
	{
	}

	auto PolarStereographicVariantB::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<PolarStereographicVariantB>(_parameters, !_inverse);
		return _inverseTransformation;
	}

	PolarStereographicVariantC::PolarStereographicVariantC(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: PolarStereographic{parameters, inverse, Variant::C}
	{
	}

	auto PolarStereographicVariantC::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<PolarStereographicVariantC>(_parameters, !_inverse);
		return _inverseTransformation;
	}
}