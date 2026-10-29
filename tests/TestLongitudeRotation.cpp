// LongitudeRotation, the longitude rotation of the European Petroleum Survey Group (EPSG) method 9601, created from
// Well-Known Text by the math transform factory with a rotation of 2.33722917 degrees, the longitude of the Paris prime
// meridian from Greenwich.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr char const* kLongitudeRotationWkt = R"WKT(PARAM_MT["longitude_rotation",PARAMETER["rotation",2.33722917]])WKT";
}

// The transformation adds 2.33722917 degrees to the longitude and returns the latitude unchanged.
TEST(LongitudeRotation, ReturnsTheLatitudeUnchanged)
{
	auto const point = GetMathTransformFactory()->CreateFromWkt(kLongitudeRotationWkt)->Transform({ 0.45, 46.8 });
	ASSERT_EQ(2u, point.size());
	EXPECT_NEAR(2.78722917, point[0], 1e-12);
	EXPECT_EQ(46.8, point[1]);
}

// The inverse transformation subtracts 2.33722917 degrees from the longitude and, for a finite result, returns the longitude
// in [-180, 180] degrees.
TEST(LongitudeRotation, InverseReturnsTheLongitudeBetweenMinus180And180Degrees)
{
	auto const point = GetMathTransformFactory()->CreateFromWkt(kLongitudeRotationWkt)->GetInverse()->Transform({ -179.0, 46.8 });
	ASSERT_EQ(2u, point.size());
	EXPECT_NEAR(178.66277083, point[0], 1e-9);
	EXPECT_EQ(46.8, point[1]);
}
