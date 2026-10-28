// Molodensky-Badekas (Coordinate Frame, geographic 2D domain, method 9636) through the European Petroleum Survey Group
// (EPSG) catalogue. The expected values come from the geographic-to-geocentric conversion on the source ellipsoid, the
// Molodensky-Badekas formula and the geocentric-to-geographic conversion on the target ellipsoid, computed outside crskit.

#include "TestBase.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateTransformations;

// Operation 1066 (Amersfoort to ETRS89 (2)) from Amersfoort (EPSG code 4289) to the European Terrestrial Reference System
// 1989 (ETRS89, EPSG code 4258) at latitude 52.225 degrees and longitude 5.21 degrees.
TEST(MolodenskyBadekas, AmersfoortToEtrs89MatchesReferenceCoordinates)
{
	CoordinateTransformationOptions options;
	options.selectOperation = [](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return 1066; };
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const transformation = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(4289), factory->CreateCoordinateSystem(4258), options);
	EXPECT_EQ("1066", transformation->GetAuthorityCode());

	auto const transformedPoint = transformation->GetMathTransform()->Transform({ 52.225, 5.21 });
	EXPECT_NEAR(52.2240038665, transformedPoint[0], 1e-8);
	EXPECT_NEAR(5.2095910612, transformedPoint[1], 1e-8);
}
