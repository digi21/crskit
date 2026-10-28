// Geocentric coordinates and datum shifts through geocentric coordinates from a geographic coordinate reference system
// (CRS) whose prime meridian is not Greenwich. The geocentric conversion receives longitudes from Greenwich, so the same
// point in a CRS with the Paris prime meridian and in a CRS with the Greenwich prime meridian of the same datum gives the
// same result. The Paris meridian is 2.5969213 grads, 2.33722917 degrees, east of Greenwich.

#include "TestBase.h"
#include "../crskit/IUtilities.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr double kParisLongitudeInDegrees = 2.5969213 * 0.9;
	constexpr double kLatitudeInDegrees = 46.8;
	constexpr double kLongitudeFromParisInDegrees = 0.45;

	auto CreateCatalogueCoordinateSystem(int epsgCode) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(epsgCode); }

	// Applies the direct chain of Utilities::CreateTransformToGeocentric for horizontal and the unknown vertical CRS.
	auto TransformToGeocentric(std::shared_ptr<CoordinateSystem> const& horizontal, std::vector<double> point) -> std::vector<double>
	{
		std::vector<std::shared_ptr<IMathTransform>> directTransformations;
		std::vector<std::shared_ptr<IMathTransform>> inverseTransformations;
		GetUtilities()->CreateTransformToGeocentric(horizontal, GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetUnknownVerticalWkt()),
			directTransformations, inverseTransformations,
			[](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { return nullptr; },
			[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });
		for (auto const& step : directTransformations)
			point = step->Transform(point);
		return point;
	}

	auto ExpectPointCoordinatesWithinTolerance(std::vector<double> const& expectedPoint, std::vector<double> const& point, double tolerance) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerance) << coordinateIndex;
	}
}

// Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian (European Petroleum Survey Group (EPSG) code 4807,
// grads) and NTF with the Greenwich prime meridian (EPSG code 4275, degrees).
TEST(PrimeMeridianGeocentric, TransformToGeocentricAddsTheLongitudeOfTheParisMeridian)
{
	auto const geocentricPointFromParisCoordinates = TransformToGeocentric(CreateCatalogueCoordinateSystem(4807), { kLatitudeInDegrees / 0.9, kLongitudeFromParisInDegrees / 0.9, 100.0 });
	auto const geocentricPointFromGreenwichCoordinates = TransformToGeocentric(CreateCatalogueCoordinateSystem(4275), { kLatitudeInDegrees, kLongitudeFromParisInDegrees + kParisLongitudeInDegrees, 100.0 });
	ExpectPointCoordinatesWithinTolerance(geocentricPointFromGreenwichCoordinates, geocentricPointFromParisCoordinates, 1e-3);
}

// A three-dimensional World Geodetic System 1984 (WGS 84) CRS with the Paris prime meridian, defined in Well-Known Text
// version 2 (WKT2), to WGS 84 geocentric (EPSG code 4978), compared with WGS 84 3D (EPSG code 4979) to EPSG 4978.
TEST(PrimeMeridianGeocentric, CreateFromCoordinateSystemsAddsParisMeridianLongitudeBeforeGeocentricConversion)
{
	auto const geographicWithParisMeridian = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCRS["WGS 84 (Paris)",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Paris",2.5969213,ANGLEUNIT["grad",0.0157079632679489]],CS[ellipsoidal,3],AXIS["geodetic latitude (Lat)",north,ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ANGLEUNIT["degree",0.0174532925199433]],AXIS["ellipsoidal height (h)",up,LENGTHUNIT["metre",1]]])WKT");
	auto const geocentric = CreateCatalogueCoordinateSystem(4978);
	auto const geocentricPointFromParisCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geographicWithParisMeridian, geocentric)->GetMathTransform()
		->Transform({ kLatitudeInDegrees, kLongitudeFromParisInDegrees, 100.0 });
	auto const geocentricPointFromGreenwichCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCatalogueCoordinateSystem(4979), geocentric)->GetMathTransform()
		->Transform({ kLatitudeInDegrees, kLongitudeFromParisInDegrees + kParisLongitudeInDegrees, 100.0 });
	ExpectPointCoordinatesWithinTolerance(geocentricPointFromGreenwichCoordinates, geocentricPointFromParisCoordinates, 1e-3);
}

// NTF with the Paris prime meridian and NTF with the Greenwich prime meridian, both with the TOWGS84 parameters of the NTF
// datum, as the Geospatial Data Abstraction Library (GDAL) writes the two coordinate reference systems in Well-Known Text
// version 1 (WKT1), with the longitude of PRIMEM in degrees, to WGS 84 (EPSG code 4326): the datum shift through
// geocentric coordinates gives the same point.
TEST(PrimeMeridianGeocentric, Towgs84DatumShiftAddsTheLongitudeOfTheParisMeridian)
{
	auto const ntfParis = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCS["NTF (Paris)",DATUM["Nouvelle_Triangulation_Francaise_Paris",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM["Paris",2.33722917,AUTHORITY["EPSG","8903"]],UNIT["grad",0.01570796326794897],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT");
	auto const ntfGreenwich = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCS["NTF",DATUM["Nouvelle_Triangulation_Francaise",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT");
	auto const wgs84 = CreateCatalogueCoordinateSystem(4326);
	auto const wgs84GeographicPointFromParisCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(ntfParis, wgs84)->GetMathTransform()
		->Transform({ kLatitudeInDegrees / 0.9, kLongitudeFromParisInDegrees / 0.9 });
	auto const wgs84GeographicPointFromGreenwichCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(ntfGreenwich, wgs84)->GetMathTransform()
		->Transform({ kLatitudeInDegrees, kLongitudeFromParisInDegrees + kParisLongitudeInDegrees });
	ExpectPointCoordinatesWithinTolerance(wgs84GeographicPointFromGreenwichCoordinates, wgs84GeographicPointFromParisCoordinates, 1e-8);
}
