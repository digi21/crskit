#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class LongitudeRotation final
	: public IMathTransform
	{

		bool _inverse;
		int _dimensions;
		double _angle;

	public:
		LongitudeRotation(int dim, double rotation, bool inverse);
		explicit LongitudeRotation(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		LongitudeRotation(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

#pragma region IMathTransform members

	public:
		auto GetWkt() const -> std::string override;

		auto GetSourceDimension() const -> int override;

		auto GetTargetDimension() const -> int override;

		auto GetIsIdentity() const -> bool override;

		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;

		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
