// StaticCoordinateTransformationFactory::CreateFromCoordinateSystems between two geographic three-dimensional coordinate
// reference systems (CRS) when one of them has no code of the European Petroleum Survey Group (EPSG) catalogue. The tests
// use European Datum 1950 (ED50) defined in Well-Known Text version 1 (WKT1) with latitude, longitude and ellipsoidal
// height axes, the International 1924 ellipsoid and TOWGS84[-87,-98,-121], and World Geodetic System 1984 (WGS 84; EPSG
// code 4979). The expected values were computed outside crskit with the EPSG formulas of the geocentric conversion and of
// the geocentric translations.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	constexpr char const* kEd50WithTowgs84Wkt = R"WKT(GEOGCS["ED50 3D",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AXIS["Ellipsoidal height",UP]])WKT";
	constexpr char const* kEd50WithoutTowgs84Wkt = R"WKT(GEOGCS["ED50 3D",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AXIS["Ellipsoidal height",UP]])WKT";

	// Latitude and longitude in degrees, ellipsoidal height in metres.
	std::vector<double> const kEd50Point{ 40.0, -3.0, 100.0 };
	std::vector<double> const kWgs84Point{ 39.998828592800, -3.001199318117, 172.922441 };
	std::vector<double> const kLatitudeLongitudeHeightTolerances{ 1e-9, 1e-9, 1e-3 };

	auto Transform(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, std::vector<double> const& point) -> std::vector<double>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)->GetMathTransform()->Transform(point);
	}

	auto ExpectPointCoordinatesWithinTolerances(std::vector<double> const& expectedPoint, std::vector<double> const& point, std::vector<double> const& tolerances) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerances[coordinateIndex]) << coordinateIndex;
	}
}

// The transformation from ED50 with TOWGS84 to EPSG 4979 applies the TOWGS84 translations to geocentric coordinates and
// converts the result to latitude, longitude and ellipsoidal height.
TEST(Geographic3DTowgs84, Ed50ToWgs84TransformsLatitudeLongitudeAndEllipsoidalHeightUsingTowgs84)
{
	auto const ed50 = GetCoordinateSystemFactory()->CreateFromWkt(kEd50WithTowgs84Wkt);
	ASSERT_EQ(3, ed50->GetDimension());
	ExpectPointCoordinatesWithinTolerances(kWgs84Point, Transform(ed50, GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4979), kEd50Point), kLatitudeLongitudeHeightTolerances);
}

// The transformation from EPSG 4979 to ED50 with TOWGS84 applies the inverse of the geocentric translations.
TEST(Geographic3DTowgs84, Wgs84ToEd50AppliesInverseTowgs84Transformation)
{
	auto const ed50 = GetCoordinateSystemFactory()->CreateFromWkt(kEd50WithTowgs84Wkt);
	ExpectPointCoordinatesWithinTolerances(kEd50Point, Transform(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4979), ed50, kWgs84Point), kLatitudeLongitudeHeightTolerances);
}

// CreateFromCoordinateSystems throws TransformationNotFoundException when the source defines ED50 without TOWGS84
// parameters and the target is EPSG 4979.
TEST(Geographic3DTowgs84, Ed50WithoutTowgs84ToWgs84ThrowsTransformationNotFoundException)
{
	auto const ed50 = GetCoordinateSystemFactory()->CreateFromWkt(kEd50WithoutTowgs84Wkt);
	EXPECT_THROW(Transform(ed50, GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4979), kEd50Point), TransformationNotFoundException);
}

// The transformation from WGS 84 defined in Well-Known Text version 2 without an identifier and with longitude, latitude
// and ellipsoidal height axes to EPSG 4979 swaps longitude and latitude and keeps the ellipsoidal height unchanged.
TEST(Geographic3DTowgs84, Wgs84WithLongitudeFirstToWgs84SwapsLongitudeAndLatitude)
{
	auto const wgs84WithLongitudeFirst = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCRS["WGS 84 (Lon, Lat, h)",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,3],AXIS["geodetic longitude (Lon)",east,ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic latitude (Lat)",north,ANGLEUNIT["degree",0.0174532925199433]],AXIS["ellipsoidal height (h)",up,LENGTHUNIT["metre",1]]])WKT");
	ExpectPointCoordinatesWithinTolerances({ 40.0, -3.0, 100.0 }, Transform(wgs84WithLongitudeFirst, GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4979), { -3.0, 40.0, 100.0 }), kLatitudeLongitudeHeightTolerances);
}