#include "pch.h"
#include "../../CoordinateSystems/Projection.h"
#include "Afine.h"
#include "CoordinateFrameRotation.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	CoordinateFrameRotation::CoordinateFrameRotation(double dx, double dy, double dz, double rX, double rY, double rZ, double dS)
		: Affine{CreateCoordinateFrameRotationParameters(dx, dy, dz, rX, rY, rZ, dS), false}
	{
	}

	CoordinateFrameRotation::CoordinateFrameRotation(std::shared_ptr<IProjection> const& parameters)
		: Affine(CreateXyzShiftParameters(
					get<double>(parameters->GetParameter("x_axis_translation").GetValue()),
					get<double>(parameters->GetParameter("y_axis_translation").GetValue()),
					get<double>(parameters->GetParameter("z_axis_translation").GetValue())), false)
	{
	}

	CoordinateFrameRotation::CoordinateFrameRotation(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: Affine(CreateXyzShiftParameters(
					get<double>(parameters->GetParameter("x_axis_translation").GetValue()),
					get<double>(parameters->GetParameter("y_axis_translation").GetValue()),
					get<double>(parameters->GetParameter("z_axis_translation").GetValue())), inverse)
	{
	}
}
