// American Polyconic with a latitude of origin other than zero, against PROJ 9.8.1. Not part of the GIGS suite
// (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

// EPSG 5472 (Panama-Colon 1911 / Panama Polyconic): origin 8 deg 15' N, false northing 1092972.1 Clarke's yards.
TEST(AmericanPolyconic, OriginAwayFromTheEquatorMatchesProj)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(5467), factory->CreateCoordinateSystem(5472))->GetMathTransform();

	auto const projected = forward->Transform({ 9.0, -79.5 });
	EXPECT_NEAR(1180380.4886581553, projected[0], 1e-3);
	EXPECT_NEAR(1184051.3365282232, projected[1], 1e-3);

	auto const geographic = forward->GetInverse()->Transform({ 1180380.4886581553, 1184051.3365282232 });
	EXPECT_NEAR(9.0, geographic[0], 1e-9);
	EXPECT_NEAR(-79.5, geographic[1], 1e-9);
}