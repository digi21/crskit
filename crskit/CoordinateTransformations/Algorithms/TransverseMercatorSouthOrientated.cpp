#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "../../CoordinateSystems/Projection.h"
#include "MapProjectionBase.h"
#include "TransverseMercatorSouthOrientated.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	namespace
	{
		// The same parameters with a zero false easting and northing.
		auto WithoutFalseOrigin(std::shared_ptr<IProjection> const& parameters) -> std::shared_ptr<IProjection>
		{
			std::vector<Parameter> values;
			for (auto const& parameter : parameters->GetParameters())
			{
				auto const& name = parameter.GetName();
				if (0 == compareNoCase(name.c_str(), "false_easting") || 0 == compareNoCase(name.c_str(), "false_northing"))
					values.emplace_back(name, 0.0);
				else
					values.push_back(parameter);
			}
			return std::make_shared<Projection>(parameters->GetClassName(), values);
		}
	}

	TransverseMercatorSouthOrientated::TransverseMercatorSouthOrientated(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MapProjectionBase{parameters, inverse}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
		, _transverseMercator{make_shared<TransverseMercator>(WithoutFalseOrigin(parameters), false)}
	{
	}

#pragma region MapProjectionBase members
	auto TransverseMercatorSouthOrientated::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		double easting, northing;
		_transverseMercator->Geo2Proj(lambda, phi, easting, northing);
		x = _falseEasting - easting;
		y = _falseNorthing - northing;
	}

	auto TransverseMercatorSouthOrientated::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		_transverseMercator->Proj2Geo(_falseEasting - x, _falseNorthing - y, lambda, phi);
	}
#pragma endregion
#pragma region IMathTransform members
	auto TransverseMercatorSouthOrientated::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<TransverseMercatorSouthOrientated>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}