#pragma once

#include <cmath>
#include <algorithm>
#include <optional>
#include <vector>

#include "Concepts.h"
#include "Interpolation.h"

namespace CrsKit::Math
{
	// Helpers for locating the lower-left node of the cell used in bilinear grid interpolation.
	// Extracted so the (historically bug-prone) boundary handling is unit-tested in isolation,
	// independently of the file readers (NadCon / NTv2 / geoid grids).

	// Lower node index of the bilinear cell containing a coordinate expressed in grid units
	// ((value - axisOrigin) / cellSize), or std::nullopt if the coordinate is non-finite (NaN/Inf,
	// e.g. a NaN input or a zero cell size) or lies outside the axis [0, nodeCount-1].
	//
	// - Uses floor (not truncation toward zero), so a point west/north of the origin is rejected
	//   rather than silently extrapolated.
	// - Clamps the result to [0, nodeCount-2], so a point sitting exactly on (or within a tiny
	//   floating-point tolerance of) a far/near edge node is accepted -- it interpolates the edge
	//   cell with a fractional position of 0 or 1 -- instead of being rejected for lacking an upper
	//   neighbour. Without this, whether an edge point is accepted depended on FP rounding noise.
	[[nodiscard]] inline auto cellLocate(double gridCoordinate, int nodeCount) -> std::optional<int>
	{
		if (nodeCount < 2 || !std::isfinite(gridCoordinate))
			return std::nullopt;

		constexpr double tolerance = 1e-6; // a millionth of a cell: absorbs FP noise on edge points
		if (gridCoordinate < -tolerance || gridCoordinate >(nodeCount - 1) + tolerance)
			return std::nullopt;

		return std::clamp(static_cast<int>(std::floor(gridCoordinate)), 0, nodeCount - 2);
	}

	// Upper neighbour clamped into the grid, for points sitting on the last node:
	// min(lower+1, nodeCount-1). Used by grids that clamp the edge instead of rejecting it.
	[[nodiscard]] inline auto cellUpperClamped(int lower, int nodeCount) -> int
	{
		return std::min(lower + 1, nodeCount - 1);
	}

	// Bilinear interpolation over a flat, row-major grid of width x height nodes (one value per node),
	// with the query already expressed in grid units ((value - axisOrigin) / cellSize). Returns
	// std::nullopt when the point falls outside the grid or is non-finite (see cellLocate). The single
	// bilinear evaluator (Interpolations::BilinearInterpolation) shared by the flat single-value grids
	// (geoid REDNAP, NadCon); the interpolation is done in index space, which is equivalent to doing it
	// in world coordinates.
	template <Arithmetic T>
	[[nodiscard]] auto sampleBilinear(std::vector<T> const& data, int width, int height,
	                                  double gridX, double gridY) -> std::optional<double>
	{
		auto const xLoOpt = cellLocate(gridX, width);
		auto const yLoOpt = cellLocate(gridY, height);
		if (!xLoOpt || !yLoOpt)
			return std::nullopt;

		auto const xLo = *xLoOpt;
		auto const yLo = *yLoOpt;
		auto const xHi = xLo + 1;
		auto const yHi = yLo + 1;

		auto const a = data[static_cast<std::size_t>(yLo) * width + xLo];
		auto const b = data[static_cast<std::size_t>(yLo) * width + xHi];
		auto const c = data[static_cast<std::size_t>(yHi) * width + xLo];
		auto const d = data[static_cast<std::size_t>(yHi) * width + xHi];

		return Interpolations<double>::BilinearInterpolation(a, b, c, d, xLo, xHi, yLo, yHi, gridX, gridY);
	}
}
