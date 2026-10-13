// A point on the other side of the antimeridian from the central meridian of its CRS projects as PROJ does.
// Reference values from PROJ 9.8.1. Not part of the GIGS suite (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct AntimeridianCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double easting;
		double northing;
	};

	class ProjectionAcrossAntimeridian : public ::testing::TestWithParam<AntimeridianCase> {};
}

TEST_P(ProjectionAcrossAntimeridian, MatchesProj)
{
	auto const& c = GetParam();
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(c.base), factory->CreateCoordinateSystem(c.crs))->GetMathTransform();

	auto const projected = forward->Transform({ c.latitude, c.longitude });
	EXPECT_NEAR(c.easting, projected[0], 1e-3);
	EXPECT_NEAR(c.northing, projected[1], 1e-3);

	auto const geographic = forward->GetInverse()->Transform({ c.easting, c.northing });
	EXPECT_NEAR(c.latitude, geographic[0], 1e-8);
	EXPECT_NEAR(c.longitude, geographic[1], 1e-8);
}

INSTANTIATE_TEST_SUITE_P(EveryMethodWithALongitudeDifference, ProjectionAcrossAntimeridian, ::testing::Values(
	AntimeridianCase{ 3477, 4759, 51.9, 177.0, 518953.6351283922, 123582.38026526703 },     // LCC 2SP, Alaska zone 10
	AntimeridianCase{ 2155, 4169, -14.3, -170.6, 7359277.081269565, -298439.4200383814 },   // LCC 1SP, American Samoa (US ft)
	AntimeridianCase{ 3832, 4326, -10.0, 170.0, 2226389.8158654715, -1111475.1028522244 },  // Mercator (variant A), PDC Mercator
	AntimeridianCase{ 3994, 4326, 0.0, -170.0, 7572166.660266464, 0.0 },                    // Mercator (variant B)
	AntimeridianCase{ 9191, 4167, -45.0, -178.0, 544630.8981541021, -584560.5522323779 })); // Albers, NZGD2000 / NIWA Albers