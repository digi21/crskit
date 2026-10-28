#include "pch.h"
#include "ThreeCoordinateHeightTransform.h"
#include "Algorithms/Afine.h"

using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	ThreeCoordinateHeightTransform::ThreeCoordinateHeightTransform(std::shared_ptr<IMathTransform> horizontalConversion, std::shared_ptr<IMathTransform> heightTransformation)
		: _horizontalConversion{ std::move(horizontalConversion) }
		, _heightTransformation{ std::move(heightTransformation) }
	{
		if (3 != _heightTransformation->GetSourceDimension() || 1 != _heightTransformation->GetTargetDimension())
			throw DimensionMismatchException(std::format("The height transformation transforms {} coordinates into {}, not 3 coordinates into 1.",
				_heightTransformation->GetSourceDimension(), _heightTransformation->GetTargetDimension()));
		if (nullptr != _horizontalConversion && (3 != _horizontalConversion->GetSourceDimension() || 3 != _horizontalConversion->GetTargetDimension()))
			throw DimensionMismatchException(std::format("The horizontal conversion transforms {} coordinates into {}, not 3 coordinates into 3.",
				_horizontalConversion->GetSourceDimension(), _horizontalConversion->GetTargetDimension()));
	}

	auto ThreeCoordinateHeightTransform::GetWkt() const -> std::string
	{
		auto const heightWkt = nullptr == _horizontalConversion
			? _heightTransformation->GetWkt()
			: std::format("CONCAT_MT[{},{}]", _horizontalConversion->GetWkt(), _heightTransformation->GetWkt());
		return std::format("CONCAT_MT[{},PASSTHROUGH_MT[2,{}]]", Affine::TransformXyzToXyXyz()->GetWkt(), heightWkt);
	}

	auto ThreeCoordinateHeightTransform::GetSourceDimension() const -> int
	{
		return 3;
	}

	auto ThreeCoordinateHeightTransform::GetTargetDimension() const -> int
	{
		return 3;
	}

	auto ThreeCoordinateHeightTransform::GetIsIdentity() const -> bool
	{
		return false;
	}

	auto ThreeCoordinateHeightTransform::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (3 != point.size())
			throw DimensionMismatchException(std::format("The point has {} coordinates, not {}.", point.size(), GetSourceDimension()));

		auto const heightCoordinates = _heightTransformation->Transform(nullptr == _horizontalConversion ? point : _horizontalConversion->Transform(point));
		return { point[0], point[1], heightCoordinates[0] };
	}

	auto ThreeCoordinateHeightTransform::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<ThreeCoordinateHeightTransform>(_horizontalConversion, _heightTransformation->GetInverse());
	}
}
