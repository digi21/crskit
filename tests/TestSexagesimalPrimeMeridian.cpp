// Prime meridians that the European Petroleum Survey Group (EPSG) catalogue gives in sexagesimal degrees, minutes and
// seconds (EPSG unit 9110), such as Bern (EPSG code 8907): 7.26225 in the catalogue means 7 degrees, 26 minutes and 22.5
// seconds east of Greenwich, 7.4395833333 degrees.

#include "TestBase.h"

#include <numbers>
#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	constexpr int kCh1903BernCode = 4801;
	constexpr auto kBernLongitudeInDegrees = 7.0 + 26.0 / 60.0 + 22.5 / 3600.0;
}

// CH1903 (Bern) has the prime meridian of Bern.
TEST(SexagesimalPrimeMeridian, LongitudeAndUnitAreInDegrees)
{
	auto const geographicCoordinateSystem = std::dynamic_pointer_cast<GeographicCoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(kCh1903BernCode));
	ASSERT_NE(nullptr, geographicCoordinateSystem);
	auto const primeMeridian = geographicCoordinateSystem->GetPrimeMeridian();
	EXPECT_NEAR(kBernLongitudeInDegrees, primeMeridian.GetLongitude(), 1e-10);
	EXPECT_NEAR(std::numbers::pi / 180.0, primeMeridian.GetAngularUnit().GetRadiansPerUnit(), 1e-15);
	EXPECT_TRUE(geographicCoordinateSystem->GetWkt(WktVersion::Wkt1).contains("PRIMEM[\"Bern\",7.439583333")) << geographicCoordinateSystem->GetWkt(WktVersion::Wkt1);
}

// The Well-Known Text version 1 (WKT1) of CH1903 (Bern) without its AUTHORITY nodes is a coordinate reference system
// (CRS) without an authority code that shares the geodetic frame of EPSG 4801: the transformation that
// CoordinateTransformationFactory::CreateFromCoordinateSystems creates from that CRS to EPSG 4801 returns the coordinates
// of the point unchanged.
TEST(SexagesimalPrimeMeridian, CrsWithoutCodeSharesTheGeodeticFrameOfTheCatalogueCrs)
{
	auto const catalogueCrs = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(kCh1903BernCode);
	auto wkt = catalogueCrs->GetWkt(WktVersion::Wkt1);
	for (auto wktPosition = wkt.find(",AUTHORITY["); std::string::npos != wktPosition; wktPosition = wkt.find(",AUTHORITY["))
		wkt.erase(wktPosition, wkt.find(']', wktPosition) - wktPosition + 1);
	auto const crsWithoutCode = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
	ASSERT_EQ(0, crsWithoutCode->GetAuthorityCode()) << wkt;

	auto const transformedPoint = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(crsWithoutCode, catalogueCrs)->GetMathTransform()->Transform({ 46.9, -0.5 });
	EXPECT_NEAR(46.9, transformedPoint[0], 1e-12);
	EXPECT_NEAR(-0.5, transformedPoint[1], 1e-12);
}
