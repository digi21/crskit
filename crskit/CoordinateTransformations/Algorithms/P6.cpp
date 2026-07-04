#include "pch.h"
#include "../MathTransformBase.h"
#include "P6.h"

using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::Math;
using namespace std;

namespace CrsKit::CoordinateTransformations::Algorithms
{
	P6::P6(std::shared_ptr<IProjection> const& parameters)
		: MathTransformBase{parameters}
		, _xso{std::get<double>(parameters->GetParameter("bin_grid_origin_i").GetValue())}
		, _yso{std::get<double>(parameters->GetParameter("bin_grid_origin_j").GetValue())}
		, _xto{std::get<double>(parameters->GetParameter("bin_grid_origin_easting").GetValue())}
		, _yto{std::get<double>(parameters->GetParameter("bin_grid_origin_northing").GetValue())}
		, _k{std::get<double>(parameters->GetParameter("scale_factor").GetValue())}
		, _mx{std::get<double>(parameters->GetParameter("bin_width_of_i_axis").GetValue())}
		, _my{std::get<double>(parameters->GetParameter("bin_width_of_j_axis").GetValue())}
		, _theta{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("map_grid_bearing_of_bin_grid_j_axis").GetValue()))}
		, _incsx{std::get<double>(parameters->GetParameter("bin_node_increment_on_i_axis").GetValue())}
		, _incsy{std::get<double>(parameters->GetParameter("bin_node_increment_on_j_axis").GetValue())}
	{
		_cosTheta = cos(_theta);
		_sinTheta = sin(_theta);
	}

	P6::P6(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: MathTransformBase{parameters, inverse}
		, _xso{std::get<double>(parameters->GetParameter("bin_grid_origin_i").GetValue())}
		, _yso{std::get<double>(parameters->GetParameter("bin_grid_origin_j").GetValue())}
		, _xto{std::get<double>(parameters->GetParameter("bin_grid_origin_easting").GetValue())}
		, _yto{std::get<double>(parameters->GetParameter("bin_grid_origin_northing").GetValue())}
		, _k{std::get<double>(parameters->GetParameter("scale_factor").GetValue())}
		, _mx{std::get<double>(parameters->GetParameter("bin_width_of_i_axis").GetValue())}
		, _my{std::get<double>(parameters->GetParameter("bin_width_of_j_axis").GetValue())}
		, _theta{DEGREES_TO_RADIANS(std::get<double>(parameters->GetParameter("map_grid_bearing_of_bin_grid_j_axis").GetValue()))}
		, _incsx{std::get<double>(parameters->GetParameter("bin_node_increment_on_i_axis").GetValue())}
		, _incsy{std::get<double>(parameters->GetParameter("bin_node_increment_on_j_axis").GetValue())}
	{
		_cosTheta = cos(_theta);
		_sinTheta = sin(_theta);
	}

#pragma region IMathTransform members
	auto P6::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		vector<double> result;
		result.resize(this->GetTargetDimension());

		if (!_inverse)
		{
			result[0] = _xto + ((point[0] - _xso) * _cosTheta * _k * _mx / _incsx) + ((point[1] - _yso) * _sinTheta * _k * _my / _incsy);
			result[1] = _yto - ((point[0] - _xso) * _sinTheta * _k * _mx / _incsx) + ((point[1] - _yso) * _cosTheta * _k * _my / _incsy);
		}
		else
		{
			result[0] = (((point[0] - _xto) * _cosTheta - (point[1] - _yto) * _sinTheta) * (_incsx / (_k * _mx))) + _xso;
			result[1] = (((point[0] - _xto) * _sinTheta + (point[1] - _yto) * _cosTheta) * (_incsy / (_k * _my))) + _yso;
		}

		return result;
	}

	auto P6::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		return make_shared<P6>(_parameters, !_inverse);
	}
#pragma endregion
}
