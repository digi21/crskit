#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../CrsKitExport.h"
#include "../../CoordinateSystems/IProjection.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class _modeCrsKit VerticalOffset final
	: public IMathTransform
	{

		bool _inverse;
		double _offset;

	public:
		explicit VerticalOffset(double verticalOffset);
		VerticalOffset(double verticalOffset, bool inverse);
		explicit VerticalOffset(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		VerticalOffset(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

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
