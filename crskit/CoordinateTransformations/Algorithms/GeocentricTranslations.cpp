#include "pch.h"
#include "Afine.h"
#include "GeocentricTranslations.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	GeocentricTranslations::GeocentricTranslations(double dx, double dy, double dz)
		: Affine{CreateXyzShiftParameters(dx, dy, dz), false}
	{
	}

	GeocentricTranslations::GeocentricTranslations(std::shared_ptr<IProjection> const& parameters)
		: Affine(CreateXyzShiftParameters(
					get<double>(parameters->GetParameter("x_axis_translation").GetValue()),
					get<double>(parameters->GetParameter("y_axis_translation").GetValue()),
					get<double>(parameters->GetParameter("z_axis_translation").GetValue())), false)
	{
	}

	GeocentricTranslations::GeocentricTranslations(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: Affine(CreateXyzShiftParameters(
					get<double>(parameters->GetParameter("x_axis_translation").GetValue()),
					get<double>(parameters->GetParameter("y_axis_translation").GetValue()),
					get<double>(parameters->GetParameter("z_axis_translation").GetValue())), inverse)
	{
	}
}
