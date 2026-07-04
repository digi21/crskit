#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "../MathTransformBase.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class P6
	: public MathTransformBase
	{

		double _xso;
		double _yso;
		double _xto;
		double _yto;
		double _k;
		double _mx;
		double _my;
		double _theta;
		double _incsx;
		double _incsy;

		double _cosTheta;
		double _sinTheta;

	public:
		explicit P6(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		P6(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

#pragma region IMathTransform members
		auto Transform(std::vector<double> const& point) const -> std::vector<double> override;
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
