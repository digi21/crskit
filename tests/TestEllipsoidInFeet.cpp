// Geocentric coordinates from a geographic coordinate reference system (CRS) whose ellipsoid is defined in a unit other
// than the metre: Trinidad 1903 (European Petroleum Survey Group (EPSG) code 4302) uses Clarke 1858, whose semi-axes the
// catalogue gives in Clarke's feet (0.3047972654 m). The expected values come from the geographic-to-geocentric
// conversion and, for operation 1296, the geocentric translations and the geocentric-to-geographic conversion, computed
// outside crskit with the semi-axes in metres.

#include "TestBase.h"
#include "../crskit/IUtilities.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr int kTrinidad1903Code = 4302;

	auto CreateCatalogueCoordinateSystem(int code) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code); }
}

// Operation 1296 (Trinidad 1903 to World Geodetic System 1984 (WGS 84) (1), geocentric translations of -61.702 m along
// X, 284.488 m along Y and 472.052 m along Z) at latitude 10.67 degrees and longitude -61.045 degrees.
TEST(EllipsoidInFeet, Trinidad1903ToWgs84MatchesReferenceCoordinates)
{
	CoordinateTransformationOptions options;
	options.selectOperation = [](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return 1296; };
	auto const transformation = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		CreateCatalogueCoordinateSystem(kTrinidad1903Code), CreateCatalogueCoordinateSystem(4326), options);
	EXPECT_EQ("1296", transformation->GetAuthorityCode());

	auto const transformedPoint = transformation->GetMathTransform()->Transform({ 10.67, -61.045 });
	EXPECT_NEAR(10.6737062800, transformedPoint[0], 1e-8);
	EXPECT_NEAR(-61.0442346094, transformedPoint[1], 1e-8);
}

// The direct chain of Utilities::CreateTransformToGeocentric for Trinidad 1903 and the unknown vertical CRS returns X, Y
// and Z in metres for latitude 10.67 degrees, longitude -61.045 degrees and ellipsoidal height 100 m.
TEST(EllipsoidInFeet, DirectTransformationChainProducesGeocentricCoordinatesInMetres)
{
	std::vector<std::shared_ptr<IMathTransform>> directTransformations;
	std::vector<std::shared_ptr<IMathTransform>> inverseTransformations;
	GetUtilities()->CreateTransformToGeocentric(CreateCatalogueCoordinateSystem(kTrinidad1903Code),
		GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetUnknownVerticalWkt()), directTransformations, inverseTransformations,
		[](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { return nullptr; },
		[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });

	std::vector<double> point{ 10.67, -61.045, 100.0 };
	for (auto const& step : directTransformations)
		point = step->Transform(point);
	EXPECT_NEAR(3034886.5594, point[0], 1e-3);
	EXPECT_NEAR(-5485235.8891, point[1], 1e-3);
	EXPECT_NEAR(1173096.4376, point[2], 1e-3);
}
