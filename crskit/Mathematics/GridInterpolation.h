#pragma once

#include <vector>
#include <algorithm>
#include "Point.h"
#include "Interpolation.h"
#include "GridCell.h"
#include "Concepts.h"
#include "../OpenGisException.h"

namespace CrsKit::Math::Transformations
{
	///	<summary>
	///		This class represents a grid for performing bilinear interpolations.
	///	</summary>
	template <Arithmetic T>
	class GridInterpolation
	{
	protected:
		Point2D<double> _cellSize;
		Point2D<double> _origin;
		int _horizontalCellCount;
		int _verticalCellCount;
		std::vector<T> _corrections;	// row-major: _verticalCellCount * _horizontalCellCount

		// Acceso 2D a la grid plana.
		auto correction(int row, int column) -> T&
		{
			return _corrections[static_cast<size_t>(row) * _horizontalCellCount + column];
		}
		auto correction(int row, int column) const -> T const&
		{
			return _corrections[static_cast<size_t>(row) * _horizontalCellCount + column];
		}

#pragma region Constructors and destructor

	public:
		GridInterpolation() = default;

		virtual ~GridInterpolation() = default;
#pragma endregion

#pragma region Abstract methods
		///	<summary>
		///		Derived classes must implement this method to load the file with the data.
		///	</summary>
		///	<param name="gridFilePath">Path to the file to load.</param>
		///	<returns>True if loading succeeded. False otherwise.</returns>
		virtual auto Load(const char* gridFilePath) -> bool = 0;
#pragma endregion

#pragma region Implementation
		///	<summary>
		///		This method computes corrections using a bilinear algorithm.
		///		It is a parameterized method. The type <c>TBilinearInterpolator</c> must implement an <c>Interpolate</c> method that returns a type <c>T</c>
		///		according to the interpolation position.
		///	</summary>
		auto ComputeBilinearCorrections(Point2D<double> const& coordinate) const noexcept(false) -> T
		{
			// _origin is the NW corner: x grows east, y grows southward by row index, hence the
			// inverted Y when converting to grid units. sampleBilinear handles cell location, bounds,
			// NaN/Inf and the bilinear evaluation.
			auto const value = sampleBilinear(_corrections, _horizontalCellCount, _verticalCellCount,
				(coordinate.x - _origin.x) / _cellSize.x,
				(_origin.y - coordinate.y) / _cellSize.y);
			if (!value)
				throw CrsKit::CoordinateOutsideDomainException{ "Coordinate outside the interpolation grid" };

			return static_cast<T>(*value);
		}

	protected:
		///	<summary>
		///		The loader methods of derived classes will call this method once the number of cells is known in
		///		horizontal y vertical.
		///	</summary>
		auto CreateCorrectionsArray() -> void
		{
			// We request memory to store all the measurements
			_corrections.assign(static_cast<size_t>(_verticalCellCount) * _horizontalCellCount, T{});
		}
#pragma endregion
	};
}
