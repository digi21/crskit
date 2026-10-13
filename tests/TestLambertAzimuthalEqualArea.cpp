// Lambert Azimuthal Equal Area in its polar aspect and across quadrants, against PROJ 9.8.1. Not part of the GIGS
// suite (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct LaeaCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double first;
		double second;
	};

	class LambertAzimuthalEqualArea : public ::testing::TestWithParam<LaeaCase> {};
}

TEST_P(LambertAzimuthalEqualArea, MatchesProj)
{
	auto const& c = GetParam();
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(c.base), factory->CreateCoordinateSystem(c.crs))->GetMathTransform();

	auto const projected = forward->Transform({ c.latitude, c.longitude });
	EXPECT_NEAR(c.first, projected[0], 1e-3);
	EXPECT_NEAR(c.second, projected[1], 1e-3);

	auto const geographic = forward->GetInverse()->Transform({ c.first, c.second });
	EXPECT_NEAR(c.latitude, geographic[0], 1e-11);
	EXPECT_NEAR(c.longitude, geographic[1], 1e-11);
}

INSTANTIATE_TEST_SUITE_P(PolarAndOblique, LambertAzimuthalEqualArea, ::testing::Values(
	LaeaCase{ 3571, 4326, 70.0, -160.0, 759856.1953639063, -2087687.7387950355 },   // north pole, Bering Sea
	LaeaCase{ 3574, 4326, 60.0, 20.0, 2866387.8131261407, -1654909.7755102406 },    // north pole, Atlantic
	LaeaCase{ 6932, 4326, -70.0, 100.0, 2187918.7146477452, -385789.1009893824 },   // south pole, EASE-Grid 2.0 South
	LaeaCase{ 10598, 4326, 30.0, -60.0, 3734914.636048683, -1249904.7411164858 },   // oblique, point east of the origin
	LaeaCase{ 3035, 4258, 40.0, -5.0, 2005271.6752567608, 3042765.835795318 }));    // oblique, LAEA Europe (axes N, E)
// The pole itself projects to the false origin.
TEST(LambertAzimuthalEqualAreaPole, PoleProjectsToTheFalseOrigin)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	for (auto const [crs, latitude] : { std::pair{ 6932, -90.0 }, std::pair{ 3571, 90.0 } })
	{
		auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
			factory->CreateCoordinateSystem(4326), factory->CreateCoordinateSystem(crs))->GetMathTransform();
		auto const projected = forward->Transform({ latitude, 0.0 });
		EXPECT_NEAR(0.0, projected[0], 1e-6) << crs;
		EXPECT_NEAR(0.0, projected[1], 1e-6) << crs;
	}
}