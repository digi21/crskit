// Lambert Conic Conformal variants: West Orientated (9826), 1SP variant B (1102), 2SP Michigan (1051) and 2SP Belgium
// (9803). Not part of the GIGS suite (no "Test5xxx" prefix).
//
// References: PROJ 9.8.1 for 1102 and 1051; for 9826, which PROJ does not implement, PROJ's LCC 1SP with W = 2 FE - E;
// for 9803 the GN7-2 example (PROJ defines EPSG 31300 without the 29.2985" rotation).

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct LccCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double first;
		double second;
		double tolerance;
	};

	class LambertConicConformalVariants : public ::testing::TestWithParam<LccCase> {};
}

TEST_P(LambertConicConformalVariants, ForwardAndReverse)
{
	auto const& c = GetParam();
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(c.base), factory->CreateCoordinateSystem(c.crs))->GetMathTransform();

	auto const projected = forward->Transform({ c.latitude, c.longitude });
	EXPECT_NEAR(c.first, projected[0], c.tolerance);
	EXPECT_NEAR(c.second, projected[1], c.tolerance);

	auto const geographic = forward->GetInverse()->Transform({ projected[0], projected[1] });
	EXPECT_NEAR(c.latitude, geographic[0], 1e-10);
	EXPECT_NEAR(c.longitude, geographic[1], 1e-10);
}

INSTANTIATE_TEST_SUITE_P(Variants, LambertConicConformalVariants, ::testing::Values(
	// 9826, axes north and west: (N, W)
	LccCase{ 2218, 4195, 70.5, -25.505, 694.4313680290898, 56088.873475497625, 1e-3 },
	LccCase{ 2297, 4194, 82.335, -38.435, -18111.587167631485, -23312.200471776654, 1e-3 },
	// 1102
	LccCase{ 10258, 10256, 56.155, 10.435, 3557.018162892465, 128575.37113004671, 1e-3 },
	LccCase{ 9549, 9547, 45.38, 6.265, 106785.0235145053, 72005.2346979715, 1e-3 },
	// 1051 (US survey feet)
	LccCase{ 6200, 4267, 46.7, -86.93, 2017564.380262956, 455832.41090480756, 1e-3 },
	// 9803, GN7-2 example (Belge 1972 / Belge Lambert 72)
	LccCase{ 31300, 4313, 50 + 40 / 60.0 + 46.461 / 3600, 5 + 48 / 60.0 + 26.533 / 3600, 251763.20, 153034.13, 0.01 }));