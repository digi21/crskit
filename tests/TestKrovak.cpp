// Krovak (9819), Krovak (North Orientated) (1041), Krovak Modified (1042) and Krovak Modified (North Orientated) (1043).
// Not part of the GIGS suite (no "Test5xxx" prefix).
//
// 9819 and 1041 against PROJ 9.8.1. 1042 and 1043 against the GN7-2 example, which is given to the centimetre: PROJ
// hard-codes the co-latitude of the cone axis as 30 deg 17' 17.30311", while EPSG stores 30 deg 17' 17.303" for the
// modified conversions, and the two differ by 3.4 mm at the example point.

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct KrovakCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double first;
		double second;
		double tolerance;
	};

	class KrovakMethods : public ::testing::TestWithParam<KrovakCase> {};

	constexpr double kExampleLatitude = 50 + 12 / 60.0 + 32.442 / 3600;
	constexpr double kExampleLongitudeFerro = 34 + 30 / 60.0 + 59.179 / 3600;
}

TEST_P(KrovakMethods, ForwardAndReverse)
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

INSTANTIATE_TEST_SUITE_P(Variants, KrovakMethods, ::testing::Values(
	// S-JTSK (Ferro) / Krovak, axes south and west; longitude from Ferro
	KrovakCase{ 2065, 4818, 49.728, 18.372 + 17 + 2 / 3.0, 1114086.0201300464, 465323.17155833193, 1e-3 },
	// S-JTSK / Krovak East North, axes east and north
	KrovakCase{ 5514, 4156, 49.728, 18.372, -465323.1715583298, -1114086.0201300473, 1e-3 },
	// S-JTSK/05 (Ferro) / Modified Krovak, GN7-2 example
	KrovakCase{ 5224, 5229, kExampleLatitude, kExampleLongitudeFerro, 6050538.71, 5568990.91, 0.01 },
	// S-JTSK/05 (Ferro) / Modified Krovak East North: the same point with E = -W and N = -S
	KrovakCase{ 5225, 5229, kExampleLatitude, kExampleLongitudeFerro, -5568990.91, -6050538.71, 0.01 }));