#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../IMathTransform.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class GeographicToGeocentric final
	: public IMathTransform
	{

		bool _inverse;
		double _a;
		double _b;
		double _e2;
		double _f;
		double _epsilon;
		std::shared_ptr<IMathTransform> _inverseTransformation;

	public:
		GeographicToGeocentric(double semiMajorAxis, double semiMinorAxis, bool inverse);
		explicit GeographicToGeocentric(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		GeographicToGeocentric(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

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
		auto ComputeConstants() -> void;
#pragma endregion
	};
}
