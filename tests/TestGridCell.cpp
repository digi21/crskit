// Boundary tests for the bilinear grid-cell location helpers (CrsKit::Math, GridCell.h).
//
// These guard the edge cases that used to be silently wrong in the grid interpolators
// (NadCon / NTv2 / geoid grids): truncation vs floor west/north of the origin, reading one past the
// last node, rejecting a point that sits exactly on a far edge node, and the UB of
// static_cast<int>(NaN). NOT part of the GIGS conformance suite, so the suite name deliberately
// avoids the "Test5xxx" prefix used by GIGS.

#include <gtest/gtest.h>

#include <limits>

#include "../crskit/Mathematics/GridCell.h"

using namespace CrsKit::Math;

// Interior point: lower cell index is floor(gridCoord). nodeCount=5 -> nodes 0..4, cells [0,1]..[3,4].
TEST(GridCellLocation, InteriorCellUsesFloor)
{
	EXPECT_EQ(0, cellLocate(0.0, 5));
	EXPECT_EQ(0, cellLocate(0.7, 5));
	EXPECT_EQ(2, cellLocate(2.999, 5));
	EXPECT_EQ(3, cellLocate(3.2, 5));
}

// A point exactly on (or within FP tolerance of) the near/far edge node must be accepted, clamping to
// the edge cell -- not rejected for lacking an upper neighbour, and not flip-flopping with FP noise.
TEST(GridCellLocation, EdgeNodesAreInclusive)
{
	EXPECT_EQ(3, cellLocate(4.0, 5));          // exactly on the last node (4) -> last cell [3,4]
	EXPECT_EQ(3, cellLocate(4.0 + 1e-9, 5));   // FP noise just past it, within tolerance -> accepted
	EXPECT_EQ(0, cellLocate(-1e-9, 5));        // FP noise just west of the first node -> first cell
}

// Genuinely outside the axis is rejected.
TEST(GridCellLocation, OutsideIsRejected)
{
	EXPECT_FALSE(cellLocate(-0.5, 5).has_value());
	EXPECT_FALSE(cellLocate(4.5, 5).has_value());
	EXPECT_FALSE(cellLocate(5.0, 5).has_value());
	EXPECT_FALSE(cellLocate(1.0, 1).has_value()); // degenerate grid (<2 nodes)
}

// NaN/Inf (e.g. a NaN coordinate or a zero cell size producing inf) is rejected instead of hitting
// the UB of static_cast<int>(NaN).
TEST(GridCellLocation, NonFiniteIsRejectedWithoutUB)
{
	EXPECT_FALSE(cellLocate(std::numeric_limits<double>::quiet_NaN(), 100).has_value());
	EXPECT_FALSE(cellLocate(std::numeric_limits<double>::infinity(), 100).has_value());
	EXPECT_FALSE(cellLocate(-std::numeric_limits<double>::infinity(), 100).has_value());
}

// Edge-clamping variant (used by NTv2): the upper neighbour never leaves the grid.
TEST(GridCellLocation, UpperClampedStaysInsideGrid)
{
	constexpr int nodes = 5;

	EXPECT_EQ(1, cellUpperClamped(0, nodes)); // interior -> lower + 1
	EXPECT_EQ(4, cellUpperClamped(3, nodes)); // -> lower + 1
	EXPECT_EQ(4, cellUpperClamped(4, nodes)); // last node -> clamp to itself, not nodeCount
}
