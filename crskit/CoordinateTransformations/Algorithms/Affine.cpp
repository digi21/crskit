#include "pch.h"
using namespace CrsKit::Math;
#include "../../CoordinateSystems/Projection.h"
#include "Afine.h"

using namespace std;
using namespace CrsKit::CoordinateSystems;

namespace CrsKit::CoordinateTransformations::Algorithms
{
#pragma region Constructors

	Affine::Affine(std::shared_ptr<IProjection> const& parameters, bool inverse)
		: _inverse{inverse}
	{
		if (parameters->GetNumParameters() < 2)
			throw std::runtime_error("NotEnoughParametersToInstantiateAffineMathTransform");

		_rows = static_cast<unsigned int>(std::get<double>(parameters->GetParameter("num_row").GetValue()));
		_columns = static_cast<unsigned int>(std::get<double>(parameters->GetParameter("num_col").GetValue()));

		if ((2 + _rows * _columns) != parameters->GetNumParameters())
			throw std::runtime_error("IncorrectAffineParameterCount");

		_matriz = Math::Matrix{_rows, _columns};
		for (auto j = 0; j < _rows; j++)
		{
			for (auto i = 0; i < _columns; i++)
			{
				_matriz[j][i] = i == j ? 1.0 : 0.0;

				auto const parameterName = std::format("elt_{}_{}", j, i);
				_matriz[j][i] = std::get<double>(parameters->GetParameter(parameterName).GetValue());
			}
		}
	}

	Affine::Affine(int _rows, int columns, Math::Matrix const& matriz, bool inverse)
		: _matriz{matriz}
		, _rows{_rows}
		, _columns{columns}
		, _inverse{inverse}
	{
	}

	auto Affine::AffineMainDiagonalTransform(double value, unsigned int componentes) -> std::shared_ptr<Affine>
	{
		componentes++;

		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", componentes));
		parameters.push_back(Parameter("num_col", componentes));

		for (auto j = 0u; j < componentes; j++)
		{
			for (auto i = 0u; i < componentes; i++)
			{
				auto const parameterName = std::format("elt_{}_{}", j, i);
				parameters.push_back(Parameter(parameterName, i == j ? value : 0.0));
			}
		}

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}

	auto Affine::Affine2DTo3DTransform() -> std::shared_ptr<Affine>
	{
		return make_shared<Affine>(CreateAffine2DTo3DParameters(), false);
	}

	auto Affine::CreateAffine2DTo3DParameters() -> std::shared_ptr<IProjection>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 3.0));

		parameters.push_back(Parameter("elt_0_0", 1.0));
		parameters.push_back(Parameter("elt_0_1", 0.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));

		parameters.push_back(Parameter("elt_1_0", 0.0));
		parameters.push_back(Parameter("elt_1_1", 1.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));

		parameters.push_back(Parameter("elt_2_0", 0.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 0.0));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 1.0));

		return make_shared<Projection>("affine", parameters);
	}

	auto Affine::IdentityTransform() -> std::shared_ptr<Affine>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", 1.0));
		parameters.push_back(Parameter("elt_0_1", 0.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));
		parameters.push_back(Parameter("elt_0_3", 0.0));

		parameters.push_back(Parameter("elt_1_0", 0.0));
		parameters.push_back(Parameter("elt_1_1", 1.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));
		parameters.push_back(Parameter("elt_1_3", 0.0));

		parameters.push_back(Parameter("elt_2_0", 0.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 1.0));
		parameters.push_back(Parameter("elt_2_3", 0.0));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 0.0));
		parameters.push_back(Parameter("elt_3_3", 1.0));

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}

	auto Affine::TransformXyzToZxy() -> std::shared_ptr<Affine>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", 0.0));
		parameters.push_back(Parameter("elt_0_1", 0.0));
		parameters.push_back(Parameter("elt_0_2", 1.0));
		parameters.push_back(Parameter("elt_0_3", 0.0));

		parameters.push_back(Parameter("elt_1_0", 1.0));
		parameters.push_back(Parameter("elt_1_1", 0.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));
		parameters.push_back(Parameter("elt_1_3", 0.0));

		parameters.push_back(Parameter("elt_2_0", 0.0));
		parameters.push_back(Parameter("elt_2_1", 1.0));
		parameters.push_back(Parameter("elt_2_2", 0.0));
		parameters.push_back(Parameter("elt_2_3", 0.0));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 0.0));
		parameters.push_back(Parameter("elt_3_3", 1.0));

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}

	auto Affine::TransformZxyToXyz() -> std::shared_ptr<Affine>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", 0.0));
		parameters.push_back(Parameter("elt_0_1", 1.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));
		parameters.push_back(Parameter("elt_0_3", 0.0));

		parameters.push_back(Parameter("elt_1_0", 0.0));
		parameters.push_back(Parameter("elt_1_1", 0.0));
		parameters.push_back(Parameter("elt_1_2", 1.0));
		parameters.push_back(Parameter("elt_1_3", 0.0));

		parameters.push_back(Parameter("elt_2_0", 1.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 0.0));
		parameters.push_back(Parameter("elt_2_3", 0.0));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 0.0));
		parameters.push_back(Parameter("elt_3_3", 1.0));

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}

	auto Affine::TransformXyzToYxz() -> std::shared_ptr<Affine>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", 0.0));
		parameters.push_back(Parameter("elt_0_1", 1.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));
		parameters.push_back(Parameter("elt_0_3", 0.0));

		parameters.push_back(Parameter("elt_1_0", 1.0));
		parameters.push_back(Parameter("elt_1_1", 0.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));
		parameters.push_back(Parameter("elt_1_3", 0.0));

		parameters.push_back(Parameter("elt_2_0", 0.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 1.0));
		parameters.push_back(Parameter("elt_2_3", 0.0));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 0.0));
		parameters.push_back(Parameter("elt_3_3", 1.0));

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}

	auto Affine::TransformXyzToXyXyz() -> std::shared_ptr<Affine>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 6.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", 1.0));
		parameters.push_back(Parameter("elt_0_1", 0.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));
		parameters.push_back(Parameter("elt_0_3", 0.0));

		parameters.push_back(Parameter("elt_1_0", 0.0));
		parameters.push_back(Parameter("elt_1_1", 1.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));
		parameters.push_back(Parameter("elt_1_3", 0.0));

		parameters.push_back(Parameter("elt_2_0", 1.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 0.0));
		parameters.push_back(Parameter("elt_2_3", 0.0));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 1.0));
		parameters.push_back(Parameter("elt_3_2", 0.0));
		parameters.push_back(Parameter("elt_3_3", 0.0));

		parameters.push_back(Parameter("elt_4_0", 0.0));
		parameters.push_back(Parameter("elt_4_1", 0.0));
		parameters.push_back(Parameter("elt_4_2", 1.0));
		parameters.push_back(Parameter("elt_4_3", 0.0));

		parameters.push_back(Parameter("elt_5_0", 0.0));
		parameters.push_back(Parameter("elt_5_1", 0.0));
		parameters.push_back(Parameter("elt_5_2", 0.0));
		parameters.push_back(Parameter("elt_5_3", 1.0));

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}

	auto Affine::Affine2DTo3DSwapAxesTransform() -> std::shared_ptr<Affine>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 3.0));

		parameters.push_back(Parameter("elt_0_0", 0.0));
		parameters.push_back(Parameter("elt_0_1", 1.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));

		parameters.push_back(Parameter("elt_1_0", 1.0));
		parameters.push_back(Parameter("elt_1_1", 0.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));

		parameters.push_back(Parameter("elt_2_0", 0.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 0.0));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 1.0));

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}

	auto Affine::CreateAffine3DTo2DParameters() -> std::shared_ptr<IProjection>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 3.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", 1.0));
		parameters.push_back(Parameter("elt_0_1", 0.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));
		parameters.push_back(Parameter("elt_0_3", 0.0));

		parameters.push_back(Parameter("elt_1_0", 0.0));
		parameters.push_back(Parameter("elt_1_1", 1.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));
		parameters.push_back(Parameter("elt_1_3", 0.0));

		parameters.push_back(Parameter("elt_2_0", 0.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 0.0));
		parameters.push_back(Parameter("elt_2_3", 1.0));

		return make_shared<Projection>("affine", parameters);
	}

	auto Affine::Affine3DTo2DTransform() -> std::shared_ptr<Affine>
	{
		return make_shared<Affine>(CreateAffine3DTo2DParameters(), false);
	}

	auto Affine::Affine3DTo2DSwapAxesTransform() -> std::shared_ptr<Affine>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 2.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", 0.0));
		parameters.push_back(Parameter("elt_0_1", 1.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));
		parameters.push_back(Parameter("elt_0_3", 0.0));

		parameters.push_back(Parameter("elt_1_0", 1.0));
		parameters.push_back(Parameter("elt_1_1", 0.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));
		parameters.push_back(Parameter("elt_1_3", 0.0));

		parameters.push_back(Parameter("elt_2_0", 0.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 0.0));
		parameters.push_back(Parameter("elt_1_3", 1.0));

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}

	auto Affine::CreateBursaWolfParameters(double dx, double dy, double dz, double rx, double ry, double rz, double dS) -> std::shared_ptr<IProjection>
	{
		rx = DEGREES_TO_RADIANS(rx);
		ry = DEGREES_TO_RADIANS(ry);
		rz = DEGREES_TO_RADIANS(rz);

		std::vector<Parameter> parameters;

		auto const m = (1 + dS * 1E-6);

		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", m));
		parameters.push_back(Parameter("elt_0_1", -m * rz));
		parameters.push_back(Parameter("elt_0_2", m * ry));
		parameters.push_back(Parameter("elt_0_3", dx));

		parameters.push_back(Parameter("elt_1_0", m * rz));
		parameters.push_back(Parameter("elt_1_1", m));
		parameters.push_back(Parameter("elt_1_2", -m * rx));
		parameters.push_back(Parameter("elt_1_3", dy));

		parameters.push_back(Parameter("elt_2_0", -m * ry));
		parameters.push_back(Parameter("elt_2_1", m * rx));
		parameters.push_back(Parameter("elt_2_2", m));
		parameters.push_back(Parameter("elt_2_3", dz));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 0.0));
		parameters.push_back(Parameter("elt_3_3", 1.0));

		return make_shared<Projection>("affine", parameters);
	}

	auto Affine::BurwaWolf(double dx, double dy, double dz, double rx, double ry, double rz, double dS) -> std::shared_ptr<Affine>
	{
		rx = DEGREES_TO_RADIANS(rx);
		ry = DEGREES_TO_RADIANS(ry);
		rz = DEGREES_TO_RADIANS(rz);

		return make_shared<Affine>(CreateBursaWolfParameters(dx, dy, dz, rx, ry, rz, dS), false);
	}

	auto Affine::CreateXyzShiftParameters(double dx, double dy, double dz) -> std::shared_ptr<IProjection>
	{
		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", 1.0));
		parameters.push_back(Parameter("elt_0_1", 0.0));
		parameters.push_back(Parameter("elt_0_2", 0.0));
		parameters.push_back(Parameter("elt_0_3", dx));

		parameters.push_back(Parameter("elt_1_0", 0.0));
		parameters.push_back(Parameter("elt_1_1", 1.0));
		parameters.push_back(Parameter("elt_1_2", 0.0));
		parameters.push_back(Parameter("elt_1_3", dy));

		parameters.push_back(Parameter("elt_2_0", 0.0));
		parameters.push_back(Parameter("elt_2_1", 0.0));
		parameters.push_back(Parameter("elt_2_2", 1.0));
		parameters.push_back(Parameter("elt_2_3", dz));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 0.0));
		parameters.push_back(Parameter("elt_3_3", 1.0));

		return make_shared<Projection>("affine", parameters);
	}

	auto Affine::CreateCoordinateFrameRotationParameters(double dx, double dy, double dz, double rx, double ry, double rz, double ds) -> std::shared_ptr<IProjection>
	{
		rx = DEGREES_TO_RADIANS(rx);
		ry = DEGREES_TO_RADIANS(ry);
		rz = DEGREES_TO_RADIANS(rz);
		auto const m = (1 + ds * 1E-6);

		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", 4.0));
		parameters.push_back(Parameter("num_col", 4.0));

		parameters.push_back(Parameter("elt_0_0", m));
		parameters.push_back(Parameter("elt_0_1", m * rz));
		parameters.push_back(Parameter("elt_0_2", -m * ry));
		parameters.push_back(Parameter("elt_0_3", dx));

		parameters.push_back(Parameter("elt_1_0", -m * rz));
		parameters.push_back(Parameter("elt_1_1", m));
		parameters.push_back(Parameter("elt_1_2", m * rx));
		parameters.push_back(Parameter("elt_1_3", dy));

		parameters.push_back(Parameter("elt_2_0", m * ry));
		parameters.push_back(Parameter("elt_2_1", -m * rx));
		parameters.push_back(Parameter("elt_2_2", m));
		parameters.push_back(Parameter("elt_2_3", dz));

		parameters.push_back(Parameter("elt_3_0", 0.0));
		parameters.push_back(Parameter("elt_3_1", 0.0));
		parameters.push_back(Parameter("elt_3_2", 0.0));
		parameters.push_back(Parameter("elt_3_3", 1.0));

		return make_shared<Projection>("affine", parameters);
	}

	auto Affine::CoordinateFrameRotation(double dx, double dy, double dz, double rx, double ry, double rz, double dS) -> std::shared_ptr<Affine>
	{
		return make_shared<Affine>(CreateCoordinateFrameRotationParameters(dx, dy, dz, rx, ry, rz, dS), false);
	}


	auto Affine::AffineSwapAxesTransform(unsigned int componentes) -> std::shared_ptr<Affine>
	{
		componentes++;

		std::vector<Parameter> parameters;
		parameters.push_back(Parameter("num_row", static_cast<double>(componentes)));
		parameters.push_back(Parameter("num_col", static_cast<double>(componentes)));

		for (unsigned int j = 0; j < componentes - 1; j++)
		{
			for (unsigned int i = 0; i < componentes; i++)
			{
				auto text = std::format("elt_{}_{}", j, i);
				parameters.push_back(Parameter(text, i == (componentes - j - 2) ? 1.0 : 0.0));
			}
		}

		for (unsigned int i = 0; i < componentes; i++)
		{
			auto text = std::format("elt_{}_{}", componentes - 1, i);
			parameters.push_back(Parameter(text, i == (componentes - 1) ? 1.0 : 0.0));
		}

		return make_shared<Affine>(make_shared<Projection>("affine", parameters), false);
	}
#pragma endregion

	auto Affine::GetWkt() const -> std::string
	{
		auto result = std::format("PARAM_MT[\"Affine\", PARAMETER[\"num_row\",{}], PARAMETER[\"num_col\", {}]",
						_rows,
						_columns);

		for (auto row = 0; row < _rows; row++)
		{
			for (auto column = 0; column < _columns; column++)
			{
				result += std::format(",PARAMETER[\"elt_{}_{}\", {:.16g}]",
							row,
							column,
							_matriz[row][column]);
			}
		}

		result += "]";
		if (!_inverse)
			return result;

		return "INVERSE_MT[" + result + "]";
	}

	auto Affine::GetSourceDimension() const -> int
	{
		return _columns - 1;
	}

	auto Affine::GetTargetDimension() const -> int
	{
		return _rows - 1;
	}

	auto Affine::GetIsIdentity() const -> bool
	{
		if (_rows != _columns)
			return false;

		for (auto row = 0; row < _rows; row++)
		{
			for (auto column = 0; column < _columns; column++)
			{
				if (row != column)
				{
					if (_matriz[row][column] != 0.0)
						return false;
				}
				else
				{
					if (_matriz[row][column] != 1.0)
						return false;
				}
			}
		}
		return true;
	}

	auto Affine::Transform(std::vector<double> const& point) const -> std::vector<double>
	{
		if (point.size() != this->GetSourceDimension())
			throw DimensionMismatchException(std::format("The point passed by parameters does not have {} dimensions", this->GetSourceDimension()));

		std::vector<double> result;
		result.resize(this->GetTargetDimension());

		for (auto j = 0; j < this->GetTargetDimension(); j++)
		{
			result[j] = _matriz[j][this->GetSourceDimension()];

			for (auto i = 0; i < this->GetSourceDimension(); i++)
				result[j] += _matriz[j][i] * point[i];
		}

		return result;
	}

	auto Affine::TransformPoints(std::span<double const> sourcePoints) const -> std::vector<double>
	{
		auto const sourceDimension = this->GetSourceDimension();
		auto const targetDimension = this->GetTargetDimension();

		if (sourceDimension == 0 || sourcePoints.size() % static_cast<size_t>(sourceDimension) != 0)
			throw DimensionMismatchException(std::format("The point buffer length is not a multiple of {} dimensions", sourceDimension));

		auto const count = sourcePoints.size() / static_cast<size_t>(sourceDimension);

		std::vector<double> result(count * static_cast<size_t>(targetDimension));
		for (size_t p = 0; p < count; ++p)
		{
			auto const* in = sourcePoints.data() + p * static_cast<size_t>(sourceDimension);
			auto* out = result.data() + p * static_cast<size_t>(targetDimension);

			for (auto j = 0; j < targetDimension; j++)
			{
				auto value = _matriz[j][sourceDimension];
				for (auto i = 0; i < sourceDimension; i++)
					value += _matriz[j][i] * in[i];
				out[j] = value;
			}
		}

		return result;
	}

	auto Affine::GetInverse() -> std::shared_ptr<IMathTransform>
	{
		if (nullptr == _inverseTransformation)
		{
			if (_rows != _columns)
				throw std::runtime_error("CannotComputeAffineInverseNonSquareMatrix");

			auto const matrizI = _matriz.ComputeInverse();
			_inverseTransformation = make_shared<Affine>(_columns, _rows, matrizI, false);
		}

		return _inverseTransformation;
	}
#pragma endregion
}
