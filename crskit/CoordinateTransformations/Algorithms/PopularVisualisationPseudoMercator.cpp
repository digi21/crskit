#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "PopularVisualisationPseudoMercator.h"

using namespace CrsKit::CoordinateSystems;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	PopularVisualisationPseudoMercator::PopularVisualisationPseudoMercator(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _latitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_origin").GetValue()))}
		, _longitudeOfNaturalOrigin{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("central_meridian").GetValue()))}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("scale_factor").GetValue())}
	{
	}

#pragma region MapProjectionBase members
	auto PopularVisualisationPseudoMercator::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		x = _falseEasting + _semiMajorAxis * (lambda - _longitudeOfNaturalOrigin);
		y = _falseNorthing + _semiMajorAxis * log(tan(M_PI / 4 + phi / 2));
	}

	auto PopularVisualisationPseudoMercator::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto d = (_falseNorthing - y) / _semiMajorAxis;

		phi = M_PI / 2 - 2 * atan(pow(M_E, d));
		lambda = ((x - _falseEasting) / _semiMajorAxis) + _longitudeOfNaturalOrigin;
	}
#pragma endregion
#pragma region IMathTransform members
	auto PopularVisualisationPseudoMercator::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<PopularVisualisationPseudoMercator>(_parameters, !_inverse);

		return _inverseTransformation;
	}
#pragma endregion
}
