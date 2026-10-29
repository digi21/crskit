// StaticCoordinateTransformationFactory::CreateFromCoordinateSystems between geocentric coordinate reference systems (CRS)
// when at least one of them has no code of the European Petroleum Survey Group (EPSG) catalogue.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	auto CreateEpsgCoordinateSystem(int epsgCode) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(epsgCode);
	}

	auto ExpectPointCoordinatesWithinTolerance(std::vector<double> const& expectedPoint, std::vector<double> const& point, double tolerance) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerance) << coordinateIndex;
	}
}
// The transformation from a World Geodetic System 1984 (WGS 84) geocentric CRS without EPSG code and with coordinates in
// kilometres to WGS 84 geocentric (EPSG code 4978) multiplies the coordinates by 1000, and its inverse divides them by 1000.
TEST(GeocentricWithoutEpsgCode, Wgs84GeocentricInKilometresToWgs84GeocentricConvertsKilometresToMetres)
{
	auto const geocentricInKilometres = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOCCS["WGS 84 geocentric (km)",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["kilometre",1000])WKT");
	auto const mathTransform = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geocentricInKilometres, CreateEpsgCoordinateSystem(4978))->GetMathTransform();
	std::vector<double> const pointInKilometres{ 4886.078813811, -256.068540034, 4078.049850961 };
	std::vector<double> const pointInMetres{ 4886078.813811, -256068.540034, 4078049.850961 };
	ExpectPointCoordinatesWithinTolerance(pointInMetres, mathTransform->Transform(pointInKilometres), 1e-3);
	ExpectPointCoordinatesWithinTolerance(pointInKilometres, mathTransform->GetInverse()->Transform(pointInMetres), 1e-6);
}

// The transformation from a European Datum 1950 (ED50) geocentric CRS without EPSG code and with TOWGS84[-87,-98,-121],
// the parameters of the transformation from ED50 to WGS 84, to WGS 84 geocentric (EPSG code 4978) adds the translations
// -87, -98 and -121 metres to the geocentric coordinates.
TEST(GeocentricWithoutEpsgCode, Ed50GeocentricWithTowgs84ToWgs84GeocentricAppliesTowgs84)
{
	auto const ed50Geocentric = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOCCS["ED50 geocentric",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["metre",1])WKT");
	auto const mathTransform = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(ed50Geocentric, CreateEpsgCoordinateSystem(4978))->GetMathTransform();
	std::vector<double> const ed50Point{ 4886299.729795, -256080.117750, 4078118.084125 };
	ExpectPointCoordinatesWithinTolerance({ 4886212.729795, -256178.117750, 4077997.084125 }, mathTransform->Transform(ed50Point), 1e-3);
	ExpectPointCoordinatesWithinTolerance(ed50Point, mathTransform->GetInverse()->Transform({ 4886212.729795, -256178.117750, 4077997.084125 }), 1e-3);
}
