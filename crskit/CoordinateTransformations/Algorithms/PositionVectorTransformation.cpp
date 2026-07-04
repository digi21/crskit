#include "pch.h"
#include "Afine.h"
#include "PositionVectorTransformation.h"

using namespace CrsKit::CoordinateSystems;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	PositionVectorTransformation::PositionVectorTransformation(double dX, double dY, double dZ, double Rx, double Ry, double Rz, double dS)
		: Affine{CreateBursaWolfParameters(dX, dY, dZ, Rx, Ry, Rz, dS), false}
	{
	}

	PositionVectorTransformation::PositionVectorTransformation(std::shared_ptr<IProjection> const& parameters)
		: Affine(CreateBursaWolfParameters(
					std::get<double>(parameters->GetParameter("x_axis_translation").GetValue()),
					std::get<double>(parameters->GetParameter("y_axis_translation").GetValue()),
					std::get<double>(parameters->GetParameter("z_axis_translation").GetValue()),
					std::get<double>(parameters->GetParameter("x_axis_rotation").GetValue()),
					std::get<double>(parameters->GetParameter("y_axis_rotation").GetValue()),
					std::get<double>(parameters->GetParameter("z_axis_rotation").GetValue()),
					std::get<double>(parameters->GetParameter("scale_difference").GetValue())),
				false)
	{
	}

	PositionVectorTransformation::PositionVectorTransformation(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: Affine(CreateBursaWolfParameters(
					std::get<double>(parameters->GetParameter("x_axis_translation").GetValue()),
					std::get<double>(parameters->GetParameter("y_axis_translation").GetValue()),
					std::get<double>(parameters->GetParameter("z_axis_translation").GetValue()),
					std::get<double>(parameters->GetParameter("x_axis_rotation").GetValue()),
					std::get<double>(parameters->GetParameter("y_axis_rotation").GetValue()),
					std::get<double>(parameters->GetParameter("z_axis_rotation").GetValue()),
					std::get<double>(parameters->GetParameter("scale_difference").GetValue())),
				inverse)
	{
	}
}
