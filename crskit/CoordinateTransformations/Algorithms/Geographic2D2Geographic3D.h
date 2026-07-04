#pragma once

#include <memory>

#include "../../CoordinateSystems/IProjection.h"
#include "Afine.h"

namespace CrsKit::CoordinateTransformations::Algorithms
{
	class Geographic2D2Geographic3D
	: public Affine
	{
		bool _inverse;

	public:
		Geographic2D2Geographic3D();
		explicit Geographic2D2Geographic3D(bool inverse);
		explicit Geographic2D2Geographic3D(std::shared_ptr<CoordinateSystems::IProjection> const& parameters);
		Geographic2D2Geographic3D(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);

		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
	};
}
