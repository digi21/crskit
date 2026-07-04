#include "pch.h"
#include "VerticalOffset.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	VerticalOffset::VerticalOffset(double verticalOffset)
		: _inverse{false}
		, _offset{verticalOffset}
	{
	}

	VerticalOffset::VerticalOffset(double verticalOffset, bool inverse)
		: _inverse{inverse}
		, _offset{verticalOffset}
	{
	}

	VerticalOffset::VerticalOffset(std::shared_ptr<IProjection> const& parameters)
		: _inverse{false}
		, _offset{std::get<double>(parameters->GetParameter("vertical_offset").GetValue())}
	{
	}

	VerticalOffset::VerticalOffset(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: _inverse{inverse}
		, _offset{std::get<double>(parameters->GetParameter("vertical_offset").GetValue())}
	{
	}

#pragma region IMathTransform members
	auto VerticalOffset::GetWkt() const -> std::string
	{
		if (_inverse)
			return std::format(
				"INVERSE_MT[PARAM_MT[\"vertical_offset\", PARAMETER[\"vertical_offset\",{:f}]]]",
				_offset);

		return std::format(
			"PARAM_MT[\"vertical_offset\", PARAMETER[\"vertical_offset\",{:f}]]",
			_offset);
	}

	auto VerticalOffset::GetSourceDimension() const -> int
	{
		return 1;
	}

	auto VerticalOffset::GetTargetDimension() const -> int
	{
		return 1;
	}

	auto VerticalOffset::GetIsIdentity() const -> bool
	{
		return 0.0 == _offset;
	}

	auto VerticalOffset::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		vector<double> result;
		result.resize(this->GetTargetDimension());

		if (!_inverse)
		{
			result[0] = point[0] + _offset;
		}
		else
		{
			result[0] = point[0] - _offset;
		}

		return result;
	}

	auto VerticalOffset::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<VerticalOffset>(_offset, !_inverse);
	}
#pragma endregion
}
