#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../CrsKitExport.h"
#include "IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	// Receives three coordinates and returns the first two unchanged and, as the third, the height that heightTransformation,
	// a transformation from three coordinates to one, returns for the three coordinates. horizontalConversion, when it is not
	// null, converts the three coordinates before heightTransformation receives them, and must change only the first two,
	// such as their order or their angular unit. GetInverse returns a ThreeCoordinateHeightTransform that keeps
	// horizontalConversion and uses heightTransformation->GetInverse() to compute the third coordinate.
	class _modeCrsKit ThreeCoordinateHeightTransform : public IMathTransform
	{
		std::shared_ptr<IMathTransform> _horizontalConversion;
		std::shared_ptr<IMathTransform> _heightTransformation;

	public:
		ThreeCoordinateHeightTransform(std::shared_ptr<IMathTransform> horizontalConversion, std::shared_ptr<IMathTransform> heightTransformation);

		// Returns the Well-Known Text (WKT) of the equivalent chain TransformXyzToXyXyz, PASSTHROUGH_MT[2, ...], which receives
		// the same input coordinates and returns the same output coordinates as ThreeCoordinateHeightTransform::Transform.
		auto GetWkt() const -> std::string override;
		auto GetSourceDimension() const -> int override;
		auto GetTargetDimension() const -> int override;
		auto GetIsIdentity() const -> bool override;
		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};
}
