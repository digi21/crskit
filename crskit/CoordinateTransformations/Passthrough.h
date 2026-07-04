#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	//[OpenGisMathTransform(ClassificationName = "Passthrough")]
	class _modeCrsKit PassThrough : public IMathTransform
	{
#pragma region Private fields
		int _firstAffectedOrdinate;
		std::shared_ptr<IMathTransform> _mathTransform;
#pragma endregion
#pragma region Constructors

	public:
		PassThrough(int firstAffectedOrdinate, std::shared_ptr<IMathTransform> const& mathTransform);

#pragma endregion
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
