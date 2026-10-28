// The tests check that StaticCoordinateTransformationFactory::CreateFromCoordinateSystems transforms coordinates from a
// geographic coordinate reference system (CRS) of World Geodetic System 1984 (WGS 84) defined in Well-Known Text (WKT)
// without an authority code to WGS 84 of the catalogue of the European Petroleum Survey Group (EPSG; code 4326). The WKT names the datum WGS_1984, D_WGS_1984 or
// World Geodetic System 1984, and the catalogue names it "World Geodetic System 1984 ensemble". The WKT CRS have the axes
// longitude and latitude, and EPSG 4326 has the axes latitude and longitude, so the transformation swaps the coordinates.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	auto TransformToEpsg4326(char const* wkt, std::vector<double> const& point) -> std::vector<double>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(GetCoordinateSystemFactory()->CreateFromWkt(wkt), GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326))
			->GetMathTransform()->Transform(point);
	}

	auto ExpectLatitude40DegreesAndLongitudeMinus3Degrees(std::vector<double> const& point) -> void
	{
		ASSERT_EQ(2u, point.size());
		EXPECT_NEAR(40.0, point[0], 1e-12);
		EXPECT_NEAR(-3.0, point[1], 1e-12);
	}
}

// WKT version 1 of ESRI, with the datum D_WGS_1984.
TEST(Wgs84DatumNames, EsriWgs84ToEpsg4326SwapsLongitudeAndLatitude)
{
	ExpectLatitude40DegreesAndLongitudeMinus3Degrees(TransformToEpsg4326(
		R"WKT(GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]])WKT",
		{ -3.0, 40.0 }));
}

// WKT version 1 with the datum WGS_1984 and without AUTHORITY nodes.
TEST(Wgs84DatumNames, Wkt1Wgs84WithoutAuthorityToEpsg4326SwapsLongitudeAndLatitude)
{
	ExpectLatitude40DegreesAndLongitudeMinus3Degrees(TransformToEpsg4326(
		R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]])WKT",
		{ -3.0, 40.0 }));
}

// WKT version 2 with the datum World Geodetic System 1984 and without ID nodes.
TEST(Wgs84DatumNames, Wkt2Wgs84WithoutIdentifierToEpsg4326SwapsLongitudeAndLatitude)
{
	ExpectLatitude40DegreesAndLongitudeMinus3Degrees(TransformToEpsg4326(
		R"WKT(GEOGCRS["WGS 84 (Lon, Lat)",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic longitude (Lon)",east,ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic latitude (Lat)",north,ANGLEUNIT["degree",0.0174532925199433]]])WKT",
		{ -3.0, 40.0 }));
}