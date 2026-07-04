#include "pch.h"
using namespace CrsKit::Math;
#include "GeocentricToTopocentric.h"
#include "Geographic2D2Geocentric.h"

using namespace CrsKit::CoordinateSystems;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	GeocentricToTopocentric::GeocentricToTopocentric(double semiMajorAxis, double semiMinorAxis, double geocentricXOfTopocentricOrigin, double geocentricYOfTopocentricOrigin, double geocentricZOfTopocentricOrigin, bool inverse)
		: _inverse{inverse}
		, _a{semiMajorAxis}
		, _b{semiMinorAxis}
		, _x0{geocentricXOfTopocentricOrigin}
		, _y0{geocentricYOfTopocentricOrigin}
		, _z0{geocentricZOfTopocentricOrigin}
	{
		ComputeConstants();
	}

	GeocentricToTopocentric::GeocentricToTopocentric(std::shared_ptr<IProjection> const& parameters)
		: _inverse{0 == compareNoCase(parameters->GetClassName().c_str(), "Topocentric_To_Geographic")}
		, _a{std::get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _b{std::get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _x0{std::get<double>(parameters->GetParameter("geocentric_x_of_topocentric_origin").GetValue())}
		, _y0{std::get<double>(parameters->GetParameter("geocentric_y_of_topocentric_origin").GetValue())}
		, _z0{std::get<double>(parameters->GetParameter("geocentric_z_of_topocentric_origin").GetValue())}
	{
		ComputeConstants();
	}

	// Note: "inverse" is NOT used but must be here for compatibility with the framework when loading from Wkt, which requires classes to have a constructor
	// with two parameters.
	GeocentricToTopocentric::GeocentricToTopocentric(std::shared_ptr<IProjection> const& parameters, [[maybe_unused]] bool inverse)
		: _inverse{0 == compareNoCase(parameters->GetClassName().c_str(), "Topocentric_To_Geographic")}
		, _a{get<double>(parameters->GetParameter("semi_major").GetValue())}
		, _b{get<double>(parameters->GetParameter("semi_minor").GetValue())}
		, _x0{get<double>(parameters->GetParameter("geocentric_x_of_topocentric_origin").GetValue())}
		, _y0{get<double>(parameters->GetParameter("geocentric_y_of_topocentric_origin").GetValue())}
		, _z0{get<double>(parameters->GetParameter("geocentric_z_of_topocentric_origin").GetValue())}
	{
		ComputeConstants();
	}

	auto GeocentricToTopocentric::ComputeConstants() const -> void
	{
		GeographicToGeocentric const geocentricToGeographicTransform(_a, _b, true);

		vector<double> geocentric;
		geocentric.resize(3);

		geocentric[0] = _x0;
		geocentric[1] = _y0;
		geocentric[2] = _z0;
		auto const geographic = geocentricToGeographicTransform.Transform(geocentric);
		auto const lambdaO = DEGREES_TO_RADIANS(geographic[0]);
		auto const phiO = DEGREES_TO_RADIANS(geographic[1]);

		_rot[0][0] = -sin(lambdaO);
		_rot[0][1] = -sin(phiO) * cos(lambdaO);
		_rot[0][2] = cos(phiO) * cos(lambdaO);
		_rot[1][0] = cos(lambdaO);
		_rot[1][1] = -sin(phiO) * sin(lambdaO);
		_rot[1][2] = cos(phiO) * sin(lambdaO);
		_rot[2][0] = 0.0;
		_rot[2][1] = cos(phiO);
		_rot[2][2] = sin(phiO);
	}

#pragma region IMathTransform members
	auto GeocentricToTopocentric::GetWkt() const -> std::string
	{
		if (_inverse)
			return std::format("PARAM_MT[\"Topocentric_To_Geographic\", PARAMETER[\"semi_major\",{:.16g}], PARAMETER[\"semi_minor\", {:.16g}], PARAMETER[\"geocentric_x_of_topocentric_origin\", {:.16g}], PARAMETER[\"geocentric_y_of_topocentric_origin\", {:.16g}], PARAMETER[\"geocentric_z_of_topocentric_origin\", {:.16g}]]", _a, _b, _x0, _y0, _z0);

		return std::format("PARAM_MT[\"Geographic_To_Topocentric\", PARAMETER[\"semi_major\",{:.16g}], PARAMETER[\"semi_minor\", {:.16g}], PARAMETER[\"geocentric_x_of_topocentric_origin\", {:.16g}], PARAMETER[\"geocentric_y_of_topocentric_origin\", {:.16g}], PARAMETER[\"geocentric_z_of_topocentric_origin\", {:.16g}]]", _a, _b, _x0, _y0, _z0);
	}

	auto GeocentricToTopocentric::GetSourceDimension() const -> int
	{
		return 3;
	}

	auto GeocentricToTopocentric::GetTargetDimension() const -> int
	{
		return 3;
	}

	auto GeocentricToTopocentric::GetIsIdentity() const -> bool
	{
		return false;
	}

	auto GeocentricToTopocentric::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() < this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		vector<double> result;
		result.resize(this->GetTargetDimension());

		if (!_inverse)
		{
			auto const x = (point[0] - _x0);
			auto const y = (point[1] - _y0);
			auto const z = (point[2] - _z0);

			for (auto i = 0; i < 3; i++)
				result[i] = _rot[0][i] * x + _rot[1][i] * y + _rot[2][i] * z;
		}
		else
		{
			for (auto i = 0; i < 3; i++)
				result[i] = _rot[i][0] * point[0] + _rot[i][1] * point[1] + _rot[i][2] * point[2];

			result[0] += _x0;
			result[1] += _y0;
			result[2] += _z0;
		}

		return result;
	}

	auto GeocentricToTopocentric::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
			_inverseTransformation = make_shared<GeocentricToTopocentric>(_a, _b, _x0, _y0, _z0, !_inverse);

		return _inverseTransformation;
	}
#pragma endregion
}
