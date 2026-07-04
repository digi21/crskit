#pragma once

#include <vector>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <cstddef>

namespace CrsKit::Math
{
	///	<summary>
	///		Matrix densa row-major. Rule of zero: std::vector manages the memory.
	///	</summary>
	class Matrix final
	{
		int _rows{};
		int _columns{};
		std::vector<double> _data;	// row-major, _rows * _columns, inicializado a 0

	public:
		Matrix() = default;

		explicit Matrix(int rows, int columns)
			: _rows{rows}
			, _columns{columns}
			, _data(static_cast<std::size_t>(rows) * static_cast<std::size_t>(columns), 0.0)
		{
		}

		[[nodiscard]] auto GetRows() const noexcept -> int { return _rows; }

		[[nodiscard]] auto GetColumns() const noexcept -> int { return _columns; }

		// Acceso por row: m[row][column].
		auto operator[](int row) noexcept -> double*
		{
			return _data.data() + static_cast<std::size_t>(row) * _columns;
		}

		auto operator[](int row) const noexcept -> double const*
		{
			return _data.data() + static_cast<std::size_t>(row) * _columns;
		}

		auto operator()(int row, int column) -> double&
		{
			if (row >= _rows || column >= _columns)
				throw std::out_of_range("Matrix index out of range");

			return _data[static_cast<std::size_t>(row) * _columns + column];
		}

		auto operator()(int row, int column) const -> double const&
		{
			if (row >= _rows || column >= _columns)
				throw std::out_of_range("Matrix index out of range");

			return _data[static_cast<std::size_t>(row) * _columns + column];
		}

		// Compute the inverse matrix.
		[[nodiscard]] auto ComputeInverse() const -> Matrix
		{
			if (_rows != _columns)
				throw std::runtime_error("Non-square matrix");

			Matrix mi{*this};			// working copy (modified during elimination)
			Matrix ai{_rows, _columns};

			for (auto j = 0; j < _rows; j++)
				for (auto i = 0; i < _rows; i++)
					ai[j][i] = (j == i ? 1.0 : 0.0);

			// the row acting as pivot is ipas
			for (auto ipas = 0; ipas < _rows; ipas++)
			{
				// we look for the largest element of the pivot column
				auto imax = ipas;
				for (auto ifil = ipas; ifil < _rows; ifil++)
					if (std::fabs(mi[ifil][ipas]) > std::fabs(mi[imax][ipas]))
						imax = ifil;

				// the elements of rows ipas and imax are swapped
				if (imax != ipas)
				{
					for (auto icol = 0; icol < _rows; icol++)
					{
						std::swap(ai[ipas][icol], ai[imax][icol]);
						if (icol >= ipas)
							std::swap(mi[ipas][icol], mi[imax][icol]);
					}
				}

				// we divide the pivot row by the element
				auto const pivot = mi[ipas][ipas];
				if (pivot == 0.0)
				{
					// We return an identity matrix
					for (auto j = 0; j < _rows; j++)
						for (auto i = 0; i < _rows; i++)
							ai[j][i] = (j == i ? 1.0 : 0.0);
					return ai;
				}

				for (auto icol = 0; icol < _rows; icol++)
				{
					ai[ipas][icol] /= pivot;
					if (icol >= ipas)
						mi[ipas][icol] /= pivot;
				}

				// the non-pivot rows are reduced
				for (auto ifil = 0; ifil < _rows; ifil++)
				{
					if (ifil == ipas)
						continue;

					auto const factor = mi[ifil][ipas];
					for (auto icol = 0; icol < _rows; icol++)
					{
						ai[ifil][icol] -= factor * ai[ipas][icol];
						mi[ifil][icol] -= factor * mi[ipas][icol];
					}
				}
			}

			return ai;
		}
	};
}
