// A projected CRS with west or south axes gets the formula output mapped onto them, sign included. Not part of the
// GIGS suite (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

// EPSG 8044 (Gusterberg Grid (Ferro)): Cassini-Soldner, which yields (easting, northing), with axes X south and
// Y west. Reference values from PROJ 9.8.1.
TEST(WestSouthAxes, CassiniWithSouthWestAxesMatchesProj)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(8042), factory->CreateCoordinateSystem(8044))->GetMathTransform();

	auto const projected = forward->Transform({ 48.5, 33.0 });
	EXPECT_NEAR(-51996.57805258913, projected[0], 1e-3);
	EXPECT_NEAR(-88333.86453810538, projected[1], 1e-3);

	auto const geographic = forward->GetInverse()->Transform({ -51996.57805258913, -88333.86453810538 });
	EXPECT_NEAR(48.5, geographic[0], 1e-9);
	EXPECT_NEAR(33.0, geographic[1], 1e-9);
}