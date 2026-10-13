// Colombia Urban (1052), Azimuthal Equidistant (1125), Modified Azimuthal Equidistant (9832), Guam Projection (9831) and
// Local Orthographic (1130). Not part of the GIGS suite (no "Test5xxx" prefix).
//
// 1052 and 1125 against the GN7-2 examples; the others against PROJ 9.8.1. Azimuthal Equidistant solves the geodesic
// problems with Vincenty's formulas, within 0.1 mm of the Karney algorithms PROJ uses.

#include <tuple>

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct AzimuthalCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double first;
		double second;
		double tolerance;
	};

	class AzimuthalProjections : public ::testing::TestWithParam<AzimuthalCase> {};
}

TEST_P(AzimuthalProjections, ForwardAndReverse)
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

INSTANTIATE_TEST_SUITE_P(EveryMethod, AzimuthalProjections, ::testing::Values(
	// MAGNA-SIRGAS / Bogota urban grid, GN7-2 example; axes north and east
	AzimuthalCase{ 6247, 4686, 4.8, -(74 + 15 / 60.0), 122543.174, 80859.033, 1e-3 },
	// WGS 84 / Equi7 Europe, GN7-2 example
	AzimuthalCase{ 27704, 4326, 63.0, 44.0, 6840895.297, 3382726.731, 1e-3 },
	AzimuthalCase{ 3295, 4675, 9.6, 138.186, 41894.17890578596, 65894.01339181891, 1e-3 },     // Guam 1963 / Yap Islands
	AzimuthalCase{ 3993, 4675, 13.544, 144.881, 64315.325855007795, 57917.52846426578, 1e-3 }, // Guam 1963 / Guam SPCS
	AzimuthalCase{ 10622, 6318, 37.636, -122.36, 7500.23451790547, 6849.49005909679, 1e-3 })); // Local Orthographic
// Near the antipode of the Equi7 Europe origin Vincenty's inverse does not converge: the projection still takes the shortest
// geodesic and agrees with PROJ 9.8.1 (Karney) to 1 cm, the precision of Vincenty's series over 20000 km, and the reverse
// conversion returns the point.
TEST(AzimuthalEquidistantAntipode, NearTheAntipodeOfTheOrigin)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(4326), factory->CreateCoordinateSystem(27704))->GetMathTransform();
	for (auto const [latitude, longitude, easting, northing] : { std::tuple{ -52.99, -155.99, 5311243.3116746405, 22117307.169501957 },
		std::tuple{ -52.0, -157.0, 14949317.799027141, 19786276.790247254 }, std::tuple{ -53.0, -156.5, 25819851.469355024, 2102445.086687106 } })
	{
		auto const projected = forward->Transform({ latitude, longitude });
		EXPECT_NEAR(easting, projected[0], 0.01) << latitude << " " << longitude;
		EXPECT_NEAR(northing, projected[1], 0.01) << latitude << " " << longitude;
		auto const geographic = forward->GetInverse()->Transform({ projected[0], projected[1] });
		EXPECT_NEAR(latitude, geographic[0], 1e-9) << latitude << " " << longitude;
		EXPECT_NEAR(longitude, geographic[1], 1e-9) << latitude << " " << longitude;
	}
}