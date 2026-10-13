// Cylindrical and spherical methods: Mercator (1SP) (Spherical), Equal Earth, Lambert Cylindrical Equal Area and its
// spherical form, Equidistant Cylindrical (1028, 9842) and its spherical form (1029, 9823), and Lambert Azimuthal Equal
// Area (Spherical). Reference values from PROJ 9.8.1. Not part of the GIGS suite (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct ProjectionCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double easting;
		double northing;
	};

	class CylindricalProjections : public ::testing::TestWithParam<ProjectionCase> {};
}

TEST_P(CylindricalProjections, MatchesProj)
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

INSTANTIATE_TEST_SUITE_P(EveryMethod, CylindricalProjections, ::testing::Values(
	ProjectionCase{ 3785, 4055, 34.024, 72.0, 8015003.337115697, 4032025.095196511 },    // Mercator (1SP) (Spherical)
	ProjectionCase{ 8857, 4326, 36.0, 72.0, 6263679.540294492, 4467702.045194158 },      // Equal Earth
	ProjectionCase{ 6933, 4326, 34.4, 72.0, 6947012.178064548, 4135469.994202368 },      // Lambert Cylindrical Equal Area
	ProjectionCase{ 3410, 10346, 34.4, 72.0, 6933677.577474749, 4156383.375784916 },     // Lambert Cylindrical Equal Area (Spherical)
	ProjectionCase{ 4087, 4326, 36.0, 72.0, 8015003.337115697, 3985542.670403827 },      // Equidistant Cylindrical
	ProjectionCase{ 4088, 4047, 36.0, 72.0, 8006043.514867659, 4003021.7574338294 },     // Equidistant Cylindrical (Spherical)
	ProjectionCase{ 32662, 4326, 36.0, 72.0, 8015003.337115697, 3985542.670403827 },     // Equidistant Cylindrical (Spherical) on WGS 84
	ProjectionCase{ 32663, 4326, 36.0, 72.0, 8015003.337115697, 3985542.670403827 },     // Equidistant Cylindrical (deprecated code 9842)
	ProjectionCase{ 3408, 10346, 63.0, 72.0, 2829076.697922139, -919222.7413084552 },    // Lambert Azimuthal Equal Area (Spherical), polar
	ProjectionCase{ 9311, 4267, 56.965, 4.312, 3889226.457980699, 5066186.079252892 })); // Lambert Azimuthal Equal Area (Spherical) on Clarke 1866