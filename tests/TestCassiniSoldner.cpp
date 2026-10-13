// Cassini-Soldner against PROJ 9.8.1, forward and reverse. Not part of the GIGS suite (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct CassiniCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double easting;
		double northing;
	};

	class CassiniSoldner : public ::testing::TestWithParam<CassiniCase> {};
}

TEST_P(CassiniSoldner, MatchesProj)
{
	auto const& c = GetParam();
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(c.base), factory->CreateCoordinateSystem(c.crs))->GetMathTransform();

	auto const projected = forward->Transform({ c.latitude, c.longitude });
	EXPECT_NEAR(c.easting, projected[0], 1e-3);
	EXPECT_NEAR(c.northing, projected[1], 1e-3);

	auto const geographic = forward->GetInverse()->Transform({ c.easting, c.northing });
	EXPECT_NEAR(c.latitude, geographic[0], 1e-9);
	EXPECT_NEAR(c.longitude, geographic[1], 1e-9);
}

INSTANTIATE_TEST_SUITE_P(ForwardAndReverse, CassiniSoldner, ::testing::Values(
	CassiniCase{ 28191, 4281, 33.0, 37.0, 337337.3285550182, 268663.11941986746 },   // Palestine Grid, 1.9 deg from the meridian
	CassiniCase{ 2066, 4157, 11.3, -60.5, 288452.13380049565, 206326.3900801403 })); // Mount Dillon / Tobago Grid (Clarke's links)