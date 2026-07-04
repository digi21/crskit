#include "pch.h"
using namespace CrsKit::Math;
#include "../MathTransformBase.h"
#include "MapProjectionBase.h"
#include "AlbersConicEqualArea.h"

using namespace CrsKit::CoordinateSystems;

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	AlbersConicEqualArea::AlbersConicEqualArea(std::shared_ptr<IProjection> const& parameters, bool const _inverse)
		: MapProjectionBase{parameters, _inverse}
		, _semiMinorAxis{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _semiMajorAxis{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _firstStandardParallelLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("standard_parallel1").GetValue()))}
		, _secondStandardParallelLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("standard_parallel2").GetValue()))}
		, _falseOriginLatitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("latitude_of_center").GetValue()))}
		, _falseOriginLongitude{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("longitude_of_center").GetValue()))}
		, _falseEasting{std::get<double>(parameters->GetParameter("false_easting").GetValue())}
		, _falseNorthing{std::get<double>(parameters->GetParameter("false_northing").GetValue())}
	{
		auto const f = (_semiMajorAxis - _semiMinorAxis) / _semiMajorAxis;
		_e = sqrt(2 * f - pow(f, 2));

		//#ifdef _DEBUG
		// Data from the book example, page 293
		//						semiMajorAxis = 6378206.4;
		//						e = 0.0822719;
		//						firstStandardParallelLatitude = DEGREES_TO_RADIANS(29.5);
		//						secondStandardParallelLatitude = DEGREES_TO_RADIANS(45.5);
		//						falseOriginLatitude = DEGREES_TO_RADIANS(23);
		//						falseOriginLongitude = DEGREES_TO_RADIANS(-96);
		//#endif
		_e2 = _e * _e;

		_latitude0 = _falseOriginLatitude;
		_longitude0 = _falseOriginLongitude;
		_latitude1 = _firstStandardParallelLatitude;
		_latitude2 = _secondStandardParallelLatitude;

		_sinlat0 = sin(_latitude0);
		_sinlat1 = sin(_latitude1);
		_sinlat2 = sin(_latitude2);
		_coslat1 = cos(_latitude1);
		_coslat2 = cos(_latitude2);

		_alfa0 = (1 - _e2) * (_sinlat0 / (1 - _e2 * pow(_sinlat0, 2)) - (1 / (2 * _e)) * log((1 - _e * _sinlat0) / (1 + _e * _sinlat0)));
		_alfa1 = (1 - _e2) * (_sinlat1 / (1 - _e2 * pow(_sinlat1, 2)) - (1 / (2 * _e)) * log((1 - _e * _sinlat1) / (1 + _e * _sinlat1)));
		_alfa2 = (1 - _e2) * (_sinlat2 / (1 - _e2 * pow(_sinlat2, 2)) - (1 / (2 * _e)) * log((1 - _e * _sinlat2) / (1 + _e * _sinlat2)));
		_m1 = _coslat1 / sqrt(1 - _e2 * pow(_sinlat1, 2));
		_m2 = _coslat2 / sqrt(1 - _e2 * pow(_sinlat2, 2));
		_n = (pow(_m1, 2) - pow(_m2, 2)) / (_alfa2 - _alfa1);
		_c = pow(_m1, 2) + (_n * _alfa1);

		_p0 = (_semiMajorAxis * sqrt(_c - _n * _alfa0)) / _n;
	}

#pragma region MapProjectionBase members
	auto AlbersConicEqualArea::Geo2Proj(double lambda, double phi, double& x, double& y) const -> void
	{
		auto const sinlat = sin(phi);

		auto const alfa = (1 - _e2) * (sinlat / (1 - _e2 * pow(sinlat, 2)) - (1 / (2 * _e)) * log((1 - _e * sinlat) / (1 + _e * sinlat)));

		auto const teta = _n * (lambda - _longitude0);
		auto const radicand = _c - _n * alfa; // can go slightly negative past the standard parallels (rounding)
		auto const p = _semiMajorAxis * (radicand > 0 ? sqrt(radicand) : 0.0) / _n;

		x = _falseEasting + (p * sin(teta));
		y = _falseNorthing + _p0 - (p * cos(teta));
	}

	auto AlbersConicEqualArea::Proj2Geo(double x, double y, double& lambda, double& phi) const -> void
	{
		auto const p = sqrt(pow(x - _falseEasting, 2) + pow(_p0 - (y - _falseNorthing), 2));
		auto const teta = atan((x - _falseEasting) / (_p0 - (y - _falseNorthing)));

		auto const alphaPrime = (_c - (pow(p, 2) * pow(_n, 2) / pow(_semiMajorAxis, 2))) / _n;
		auto const betaPrime = asin(clampToUnit(alphaPrime / (1 - ((1 - pow(_e, 2)) / (2 * _e)) * log((1 - _e) / (1 + _e)))));
		//The formula above is missing two closing parentheses in the document in G7-2; in PP_1395 it does not appear, and I have put them at the end although I am not sure.

		phi = betaPrime + (pow(_e, 2) / 3 + 31 * pow(_e, 4) / 180 + 517 * pow(_e, 6) / 5040) * sin(2 * betaPrime)
		+ (23 * pow(_e, 4) / 360 + 252 * pow(_e, 6) / 3780) * sin(4 * betaPrime)
		+ (761 * pow(_e, 6) / 45360) * sin(6 * betaPrime);

		lambda = _longitude0 + (teta / _n);
	}
#pragma endregion

#pragma region IMathTransform members
	auto AlbersConicEqualArea::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			_inverseTransformation = make_shared<AlbersConicEqualArea>(_parameters, !_inverse);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
