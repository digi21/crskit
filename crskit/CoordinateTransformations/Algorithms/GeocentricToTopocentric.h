#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../../Mathematics/Matrix.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class GeocentricToTopocentric final
	: public IMathTransform
	{

		bool _inverse;
		double _a, _b;
		double _x0, _y0, _z0;
		mutable Math::Matrix _rot{ 3,3 };	// cache computed lazily in ComputeConstants() const
		std::shared_ptr<IMathTransform> _inverseTransformation;

	public:
		GeocentricToTopocentric(double semiMajorAxis, double semiMinorAxis, double geocentricXOfTopocentricOrigin, double geocentricYOfTopocentricOrigin, double geocentricZOfTopocentricOrigin, bool inverse);
		explicit GeocentricToTopocentric(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		GeocentricToTopocentric(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

#pragma region IMathTransform members

	public:
		auto GetWkt() const -> std::string override;

		auto GetSourceDimension() const -> int override;

		auto GetTargetDimension() const -> int override;

		auto GetIsIdentity() const -> bool override;

		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;

		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
#pragma region Implementation

	private:
		auto ComputeConstants() const -> void;
#pragma endregion
	};
}
