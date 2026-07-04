#include "pch.h"
#include "ConcatenatedTransform.h"

using namespace std;
using namespace CrsKit::Math;

namespace CrsKit::CoordinateTransformations
{
	ConcatenatedTransform::ConcatenatedTransform(std::shared_ptr<IMathTransform> const& transform1, std::shared_ptr<IMathTransform> const& transform2)
	{
		_transformations.push_back(transform1);
		_transformations.push_back(transform2);
	}

	ConcatenatedTransform::ConcatenatedTransform(vector<shared_ptr<IMathTransform>> const& transformations)
		: _transformations{transformations}
	{
	}

	auto ConcatenatedTransform::GetWkt() const -> std::string
	{
		std::string text("CONCAT_MT[");

		auto sw = false;
		for (auto const& transformation : _transformations)
		{
			if (!sw)
			{
				sw = true;
			}
			else
			{
				text += ",";
			}

			text += transformation->GetWkt();
		}

		return text + "]";
	}

	auto ConcatenatedTransform::GetSourceDimension() const -> int
	{
		return _transformations[0]->GetSourceDimension();
	}

	auto ConcatenatedTransform::GetTargetDimension() const -> int
	{
		return _transformations[_transformations.size() - 1]->GetTargetDimension();
	}

	auto ConcatenatedTransform::GetIsIdentity() const -> bool
	{
		return std::ranges::all_of(_transformations, [](auto const& transformation) { return transformation->GetIsIdentity(); });
	}

	auto ConcatenatedTransform::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		return std::ranges::fold_left(_transformations, point,
			[](std::vector<double> const& acc, auto const& transformation) { return transformation->Transform(acc); });
	}

	auto ConcatenatedTransform::TransformPoints(std::span<double const> sourcePoints) const -> std::vector<double>
	{
		std::vector<double> buffer(sourcePoints.begin(), sourcePoints.end());
		return std::ranges::fold_left(_transformations, std::move(buffer),
			[](std::vector<double> const& acc, auto const& transformation) { return transformation->TransformPoints(acc); });
	}

	auto ConcatenatedTransform::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (auto const cached = _inverseTransformation.lock())
			return cached;

		vector<shared_ptr<IMathTransform>> inverseList;
		for (auto const& transformation : _transformations | std::views::reverse)
			inverseList.push_back(transformation->GetInverse());

		auto inverse = make_shared<ConcatenatedTransform>(inverseList);
		_inverseTransformation = inverse;
		return inverse;
	}
}
