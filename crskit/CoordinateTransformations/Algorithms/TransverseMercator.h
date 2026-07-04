#pragma once

#include <memory>
#include <vector>

#include "../../CoordinateSystems/IProjection.h"
#include "MapProjectionBase.h"

#define _FORMULAS_JHS

namespace CrsKit::CoordinateTransformations::Algorithms::MapProjections
{
	class TransverseMercator final
	: public MapProjectionBase
	{

#pragma region Parameters
		double _semiMinorAxis;
		double _semiMajorAxis;
		double _latitudeOfNaturalOrigin;
		double _longitudeOfNaturalOrigin;
		double _scaleFactorAtNaturalOrigin;
		double _falseEasting;
		double _falseNorthing;
#pragma endregion
#pragma region Constants to speed up the computation
#ifdef _FORMULAS_JHS
		double _a;
		double _b;
		double _lambdaO;
		double _phiO;
		double _fe;
		double _fn;
		double _kO;

		double _f;
		double _e;
		double _n;
		double _n2;
		double _n3;
		double _n4;
		double _B;
		double _h1;
		double _h2;
		double _h3;
		double _h4;
		double _h1P;
		double _h2P;
		double _h3P;
		double _h4P;
		double _so;
#else
		double a;
		double e;
		double e2;
		double e4;
		double e6;
		//						double e8;
		double M0;
		double e1;
		double e_2;
#endif
#pragma endregion

#pragma region Constructor

	public:
		TransverseMercator(std::shared_ptr<CoordinateSystems::IProjection> const& parameters, bool inverse);
#pragma endregion
#pragma region MapProjectionBase members
		auto Geo2Proj(double lambda, double phi, double& x, double& y) const -> void override;
		auto Proj2Geo(double x, double y, double& lambda, double& phi) const -> void override;
#pragma endregion
#pragma region IMathTransform members
		auto GetInverse() -> std::shared_ptr<IMathTransform> override;
#pragma endregion
	};
}
