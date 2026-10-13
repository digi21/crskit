// Hotine Oblique Mercator (variant B) with an azimuth of 90 deg, against PROJ 9.8.1. Not part of the GIGS suite
// (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct ObliqueMercatorCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double easting;
		double northing;
	};

	class ObliqueMercatorAzimuth90 : public ::testing::TestWithParam<ObliqueMercatorCase> {};
}

TEST_P(ObliqueMercatorAzimuth90, MatchesProj)
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

INSTANTIATE_TEST_SUITE_P(SwitzerlandAndHungary, ObliqueMercatorAzimuth90, ::testing::Values(
	ObliqueMercatorCase{ 2056, 4150, 47.41, 9.59, 2762262.0062271687, 1253092.9507483162 },   // CH1903+ / LV95, east of the centre
	ObliqueMercatorCase{ 2056, 4150, 46.21, 6.86, 2555275.806561594, 1117644.460662901 },     // CH1903+ / LV95, west of the centre
	ObliqueMercatorCase{ 23700, 4237, 47.5, 21.5, 834660.5427399104, 242428.92048935124 }));  // HD72 / EOV