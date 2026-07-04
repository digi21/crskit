#include "pch.h"
using namespace CrsKit::Math;
#include "Geographic2D2Geocentric.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	GeographicToGeocentric::GeographicToGeocentric(double semiMajorAxis, double semiMinorAxis, bool inverse)
		: _inverse{inverse}
		, _a{semiMajorAxis}
		, _b{semiMinorAxis}
	{
		ComputeConstants();
	}

	GeographicToGeocentric::GeographicToGeocentric(std::shared_ptr<IProjection> const& parameters)
		: _inverse{0 == compareNoCase(parameters->GetClassName().c_str(), "Geocentric_To_Ellipsoid")}
		, _a{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _b{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
	{
		ComputeConstants();
	}

	// Note: "inverse" is NOT used but must be here for compatibility with the framework when loading from Wkt, which requires classes to have a constructor
	// with two parameters.
	GeographicToGeocentric::GeographicToGeocentric(std::shared_ptr<IProjection> const& parameters, [[maybe_unused]] bool inverse)
		: _inverse{0 == compareNoCase(parameters->GetClassName().c_str(), "Geocentric_To_Ellipsoid")}
		, _a{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _b{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
	{
		ComputeConstants();
	}

	auto GeographicToGeocentric::ComputeConstants() -> void
	{
		_f = (_a - _b) / _a;
		_e2 = 2 * _f - _f * _f;

		_epsilon = _e2 / (1 - _e2);
		_b = _a * (1 - _f);
	}

#pragma region IMathTransform members
	auto GeographicToGeocentric::GetWkt() const -> std::string
	{
		if (_inverse)
			return std::format("PARAM_MT[\"Geocentric_To_Ellipsoid\", PARAMETER[\"semi_major\",{:.16g}], PARAMETER[\"semi_minor\", {:.16g}]]", _a, _b);

		return std::format("PARAM_MT[\"Ellipsoid_To_Geocentric\", PARAMETER[\"semi_major\",{:.16g}], PARAMETER[\"semi_minor\", {:.16g}]]", _a, _b);
	}

	auto GeographicToGeocentric::GetSourceDimension() const -> int
	{
		return 3;
	}

	auto GeographicToGeocentric::GetTargetDimension() const -> int
	{
		return 3;
	}

	auto GeographicToGeocentric::GetIsIdentity() const -> bool
	{
		return false;
	}

	auto GeographicToGeocentric::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() < this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		vector<double> result;
		result.resize(this->GetTargetDimension());

		if (!_inverse)
		{
			auto const longitude = DEGREES_TO_RADIANS(point[0]);
			auto const latitude = DEGREES_TO_RADIANS(point[1]);
			auto const h = point[2];

			auto const v = _a / sqrt(1.0 - _e2 * pow(sin(latitude), 2.0));

			result[0] = (v + h) * cos(latitude) * cos(longitude);
			result[1] = (v + h) * cos(latitude) * sin(longitude);
			result[2] = ((1.0 - _e2) * v + h) * sin(latitude);
		}
		else
		{
			auto const p = sqrt(point[0] * point[0] + point[1] * point[1]);
			auto const q = atan((point[2] * _a) / (p * _b));

			auto const latitude = atan((point[2] + _epsilon * _b * pow(sin(q), 3.0)) / (p - _e2 * _a * pow(cos(q), 3.0)));
			auto const longitude = atan2(point[1], point[0]);

			auto const v = _a / sqrt(1.0 - _e2 * pow(sin(latitude), 2.0));
			auto const h = (p / cos(latitude)) - v;

			result[0] = RADIANS_TO_DEGREES(longitude);
			result[1] = RADIANS_TO_DEGREES(latitude);
			result[2] = h;
		}

		return result;
	}

	auto GeographicToGeocentric::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<GeographicToGeocentric>(_a, _b, !_inverse);
	}
#pragma endregion
}
