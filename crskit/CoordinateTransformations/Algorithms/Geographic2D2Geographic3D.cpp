#include "pch.h"
#include "Afine.h"
#include "Geographic2D2Geographic3D.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	Geographic2D2Geographic3D::Geographic2D2Geographic3D()
		: Affine{CreateAffine2DTo3DParameters(), false}
		, _inverse{false}
	{
	}

	Geographic2D2Geographic3D::Geographic2D2Geographic3D(bool _inverse)
		: Affine{_inverse ? CreateAffine3DTo2DParameters() : CreateAffine2DTo3DParameters(), false}
		, _inverse{_inverse}
	{
	}

	Geographic2D2Geographic3D::Geographic2D2Geographic3D([[maybe_unused]] std::shared_ptr<IProjection> const& parameters)
		: Affine{CreateAffine2DTo3DParameters(), false}
		, _inverse{false}
	{
	}

	Geographic2D2Geographic3D::Geographic2D2Geographic3D([[maybe_unused]] std::shared_ptr<IProjection> const& parameters, bool const inverse)
		: Affine{inverse ? CreateAffine3DTo2DParameters() : CreateAffine2DTo3DParameters(), false}
		, _inverse{inverse}
	{
	}

	auto Geographic2D2Geographic3D::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<Geographic2D2Geographic3D>(!_inverse);
	}
}
