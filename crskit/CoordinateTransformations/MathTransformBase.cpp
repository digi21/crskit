#include "pch.h"
#include "../CoordinateSystems/Projection.h"
#include "MathTransformBase.h"

using namespace CrsKit::CoordinateSystems;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	MathTransformBase::MathTransformBase(bool _inverse)
		: _inverse{_inverse}
	{
	}

	MathTransformBase::MathTransformBase(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: _parameters{parameters}
		, _inverse{inverse}
	{
	}

	MathTransformBase::MathTransformBase(std::shared_ptr<IProjection> const& parameters)
		: _parameters{parameters}
		, _inverse{false}
	{
	}

	auto MathTransformBase::GetWkt() const -> std::string
	{
		if (!_inverse)
			return Projection::ImprimeIProjectionComoMathTransform(_parameters);

		return std::format("INVERSE_MT[{}]", Projection::ImprimeIProjectionComoMathTransform(_parameters).c_str());
	}

	auto MathTransformBase::GetSourceDimension() const -> int
	{
		return 2;
	}

	auto MathTransformBase::GetTargetDimension() const -> int
	{
		return 2;
	}

	auto MathTransformBase::GetIsIdentity() const -> bool
	{
		return false;
	}
#pragma endregion
}
