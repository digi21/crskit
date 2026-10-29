// StaticCoordinateTransformationFactory::CreateFromCoordinateSystems from a three-dimensional coordinate reference system
// (CRS) without a code of the European Petroleum Survey Group (EPSG) catalogue to a two-dimensional CRS. The first two
// tests use European Datum 1950 (ED50) defined in Well-Known Text version 2 (WKT2) with latitude, longitude and
// ellipsoidal height axes; for this source CRS, the transformation receives latitude and longitude and returns the
// coordinates of the target CRS. For the geocentric source CRS of the third test, CreateFromCoordinateSystems throws
// TransformationNotFoundException.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	constexpr char const* kEd50Geographic3DWkt = R"WKT(GEOGCRS["ED50 3D",DATUM["European Datum 1950",ELLIPSOID["International 1924",6378388,297,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,3],AXIS["geodetic latitude (Lat)",north,ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ANGLEUNIT["degree",0.0174532925199433]],AXIS["ellipsoidal height (h)",up,LENGTHUNIT["metre",1]]])WKT";

	// Latitude and longitude in degrees.
	std::vector<double> const kEd50Point{ 40.0, -3.0 };

	auto CreateMathTransform(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target) -> std::shared_ptr<CoordinateTransformations::IMathTransform>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)->GetMathTransform();
	}

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

// The transformation from ED50 3D without EPSG code to ED50 / UTM zone 30N (Universal Transverse Mercator; EPSG code 23030)
// receives two coordinates. For latitude 40 degrees and longitude -3 degrees, each projected coordinate that it returns
// differs by at most 1 millimetre from the corresponding coordinate returned by the transformation from ED50 (EPSG code
// 4230) to EPSG 23030.
TEST(GeographicAndGeocentric3DWithoutCodeToHorizontal, Ed50ToEd50UtmZone30NProjectsLatitudeAndLongitude)
{
	auto const mathTransform = CreateMathTransform(GetCoordinateSystemFactory()->CreateFromWkt(kEd50Geographic3DWkt), CreateEpsgCoordinateSystem(23030));
	EXPECT_EQ(2, mathTransform->GetSourceDimension());
	ExpectPointCoordinatesWithinTolerance(CreateMathTransform(CreateEpsgCoordinateSystem(4230), CreateEpsgCoordinateSystem(23030))->Transform(kEd50Point), mathTransform->Transform(kEd50Point), 1e-3);
}

// For latitude 40 degrees and longitude -3 degrees, the transformation from ED50 3D without EPSG code to ED50 (EPSG code
// 4230) returns each coordinate within 1e-12 degrees of its input value.
TEST(GeographicAndGeocentric3DWithoutCodeToHorizontal, Ed50ToEd50ReturnsLatitudeAndLongitudeWithinTolerance)
{
	ExpectPointCoordinatesWithinTolerance(kEd50Point, CreateMathTransform(GetCoordinateSystemFactory()->CreateFromWkt(kEd50Geographic3DWkt), CreateEpsgCoordinateSystem(4230))->Transform(kEd50Point), 1e-12);
}

// CreateFromCoordinateSystems throws TransformationNotFoundException when asked to create a transformation from a
// geocentric CRS without EPSG code to ED50 / UTM zone 30N (EPSG code 23030): the geocentric CRS has no horizontal component.
TEST(GeographicAndGeocentric3DWithoutCodeToHorizontal, GeocentricWithoutCodeToProjectedThrowsTransformationNotFoundException)
{
	auto const geocentricSourceCoordinateSystem = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOCCS["ED50 geocentric",DATUM["European Datum 1950",SPHEROID["International 1924",6378388,297]],PRIMEM["Greenwich",0],UNIT["metre",1]])WKT");
	EXPECT_THROW(static_cast<void>(CreateMathTransform(geocentricSourceCoordinateSystem, CreateEpsgCoordinateSystem(23030))), TransformationNotFoundException);
}
