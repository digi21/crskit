#include "pch.h"
#include "Passthrough.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	PassThrough::PassThrough(int _firstAffectedOrdinate, std::shared_ptr<IMathTransform> const& _mathTransform)
		: _firstAffectedOrdinate{_firstAffectedOrdinate}
		, _mathTransform{_mathTransform}
	{
	}

#pragma region IMathTransform members
	auto PassThrough::GetWkt() const -> std::string
	{
		return std::format("PASSTHROUGH_MT[{},{}]",
					_firstAffectedOrdinate,
					_mathTransform->GetWkt().c_str());
	}

	auto PassThrough::GetSourceDimension() const -> int
	{
		return _mathTransform->GetSourceDimension() + _firstAffectedOrdinate;
	}

	auto PassThrough::GetTargetDimension() const -> int
	{
		return _mathTransform->GetTargetDimension() + _firstAffectedOrdinate;
	}

	auto PassThrough::GetIsIdentity() const -> bool
	{
		return _mathTransform->GetIsIdentity();
	}

	auto PassThrough::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		auto const sourceDimension = this->GetSourceDimension();

		if (point.size() != sourceDimension)
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		vector<double> enviar;
		enviar.resize(sourceDimension - _firstAffectedOrdinate);
		for (auto i = 0; i < sourceDimension - _firstAffectedOrdinate; i++)
			enviar[i] = point[i + _firstAffectedOrdinate];

		auto const transformResult = _mathTransform->Transform(enviar);

		vector<double> result;
		result.resize(this->GetTargetDimension());
		for (auto i = 0; i < _firstAffectedOrdinate; i++)
			result[i] = point[i];
		for (auto i = 0; i < transformResult.size(); i++)
			result[i + _firstAffectedOrdinate] = transformResult[i];

		return result;
	}

	auto PassThrough::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<PassThrough>(_firstAffectedOrdinate, _mathTransform->GetInverse());
	}
#pragma endregion
}
