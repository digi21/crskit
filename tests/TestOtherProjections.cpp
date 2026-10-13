// Bonne (South Orientated) (9828), Hyperbolic Cassini-Soldner (9833), Lambert Conic Near-Conformal (9817), Transverse
// Mercator Zoned Grid System (9824), New Zealand Map Grid (9811) and Tunisia Mining Grid (9816). Not part of the GIGS
// suite (no "Test5xxx" prefix).
//
// References: PROJ 9.8.1 for 9833 and 9811; for the methods PROJ does not implement, PROJ's Bonne with W = 2 FE - E and
// S = 2 FN - N (9828), UTM zone 30N plus 30 * 10^6 (9824), and the GN7-2 examples (9817, 9816).

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct OtherCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double first;
		double second;
		double tolerance;
	};

	class OtherProjections : public ::testing::TestWithParam<OtherCase> {};
}

TEST_P(OtherProjections, ForwardAndReverse)
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

INSTANTIATE_TEST_SUITE_P(EveryMethod, OtherProjections, ::testing::Values(
	// Lisbon 1890 (Lisbon) / Portugal Bonne New, axes south and west; longitude from Lisbon
	OtherCase{ 2963, 4904, 40.076, 1.593906111111112, -45611.81814284904, -50653.20646338004, 1e-3 },
	// Vanua Levu 1915 / Vanua Levu Grid (Clarke's links). The axes are X north, Y east; crskit keeps (E, N) for axes named
	// that way (see CreateProjectionTransforms), PROJ returns (N, E).
	OtherCase{ 3139, 4748, -16.5, 179.3, 1233640.5403654424, 1525372.0496399868, 1e-3 },
	// Deir ez Zor / Levant Zone, GN7-2 example
	OtherCase{ 22700, 4227, 37 + 31 / 60.0 + 17.625 / 3600, 34 + 8 / 60.0 + 11.291 / 3600, 15707.96, 623165.96, 0.01 },
	// WGS 84 / UTM grid system (northern hemisphere): zone 30
	OtherCase{ 32600, 4326, 40.5, -3.7, 30440686.066389695, 4483490.051518481, 1e-3 },
	// NZGD49 / New Zealand Map Grid
	OtherCase{ 27200, 4272, -39.392, 173.726, 2572589.3606062783, 6201429.023627816, 1e-3 },
	// Carthage (Paris) / Tunisia Mining Grid, GN7-2 example: grid reference 302598, latitude and longitude in grads
	// from Paris, coordinates in kilometres
	OtherCase{ 22300, 4816, 38.97997, 8.22437, 302.0, 598.0, 1e-3 }));