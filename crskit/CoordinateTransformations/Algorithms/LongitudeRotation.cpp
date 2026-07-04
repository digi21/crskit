#include "pch.h"
#include "LongitudeRotation.h"

using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	LongitudeRotation::LongitudeRotation(int dim, double rotation, bool inverse)
		: _inverse{inverse}
		, _dimensions{dim}
		, _angle{DEGREES_TO_RADIANS(rotation)}
	{
	}

	LongitudeRotation::LongitudeRotation(std::shared_ptr<CoordinateSystems::IProjection> const& parameters)
		: _inverse{ false }
		, _dimensions{2} //(int)(double)parameters->GetParameter("dim").GetValue()
		, _angle{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("rotation").GetValue()))}
	{
	}

	LongitudeRotation::LongitudeRotation(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse)
		: _inverse{inverse}
		, _dimensions{2} //(int)(double)parameters->GetParameter("dim").GetValue()
		, _angle{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("rotation").GetValue()))}
	{
	}

#pragma region IMathTransform members
	auto LongitudeRotation::GetWkt() const -> std::string
	{
		if (_inverse)
			return std::format("PARAM_MT[\"longitude_rotation\", PARAMETER[\"dim\",{}], PARAMETER[\"rotation\", {:.16g}]]",
				_dimensions, RADIANS_TO_DEGREES(_angle));

		return std::format("PARAM_MT[\"longitude_rotation\", PARAMETER[\"dim\",{}], PARAMETER[\"rotation\", {:.16g}]]",
			_dimensions, RADIANS_TO_DEGREES(_angle));
	}

	auto LongitudeRotation::GetSourceDimension() const -> int
	{
		return _dimensions;
	}

	auto LongitudeRotation::GetTargetDimension() const -> int
	{
		return _dimensions;
	}

	auto LongitudeRotation::GetIsIdentity() const -> bool
	{
		return false;
	}

	auto LongitudeRotation::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		vector<double> result;
		result.resize(this->GetTargetDimension());

		auto lambda = DEGREES_TO_RADIANS(point[0]);
		auto const phi = DEGREES_TO_RADIANS(point[1]);

		// O(1) normalisation to [-pi, pi]; a while-loop hangs on huge/non-finite lambda.
		if (std::isfinite(lambda))
			lambda = std::remainder(lambda, 2 * M_PI);

		result[1] = point[1];

		if (phi < -90 || phi > 90)
			lambda = 0.0;
		else
		{
			if (_inverse)
				lambda -= _angle;
			else
				lambda += _angle;
		}

		// O(1) normalisation to [-pi, pi]; a while-loop hangs on huge/non-finite lambda.
		if (std::isfinite(lambda))
			lambda = std::remainder(lambda, 2 * M_PI);

		result[0] = RADIANS_TO_DEGREES(lambda);
		result[1] = RADIANS_TO_DEGREES(phi);
		return result;
	}

	auto LongitudeRotation::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<LongitudeRotation>(_dimensions, RADIANS_TO_DEGREES(_angle), !_inverse);
	}
#pragma endregion
}
