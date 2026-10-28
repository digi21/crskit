// The tests check that StaticCoordinateTransformationFactory::CreateFromCoordinateSystems transforms coordinates
// between geographic coordinate reference systems (CRS) of the European Petroleum Survey Group (EPSG) catalogue when
// the source CRS, the target CRS or both are three-dimensional. The catalogue defines the operations between the
// two-dimensional CRS. The transformation returns the converted ellipsoidal height when the target is three-dimensional
// and no height when the target is two-dimensional. The tests use North American Datum 1983 (Canadian Spatial Reference
// System), abbreviated NAD83(CSRS), with EPSG codes 4617 and 4955 and the Geodetic Reference System 1980 (GRS 1980)
// ellipsoid, and World Geodetic System 1984 (WGS 84; EPSG codes 4326 and 4979), with the operation EPSG 1946
// (coordinate frame rotation). The expected values were computed outside crskit with the EPSG formulas for the
// geocentric conversion and the coordinate frame rotation.

#include "TestBase.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr int kNad83CsrsToWgs84OperationCode = 1946;

	auto TransformPointWithEpsgOperation1946(int sourceEpsgCode, int targetEpsgCode, std::vector<double> const& point) -> std::vector<double>
	{
		CoordinateTransformationOptions options;
		options.selectOperation = [](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return kNad83CsrsToWgs84OperationCode; };
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(sourceEpsgCode),
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(targetEpsgCode),
			options)->GetMathTransform()->Transform(point);
	}

	auto ExpectPointCoordinatesWithinTolerances(std::vector<double> const& expectedPoint, std::vector<double> const& point, std::vector<double> const& tolerances) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerances[coordinateIndex]) << coordinateIndex;
	}

	// Latitude and longitude in degrees, ellipsoidal height in metres.
	std::vector<double> const kNad83CsrsPoint{ 45.0, -75.0, 100.0 };
	std::vector<double> const kWgs84Point{ 45.000009002217, -75.000001910773, 98.875774 };
	std::vector<double> const kLatitudeLongitudeHeightTolerances{ 1e-9, 1e-9, 1e-3 };
	// Latitude and longitude of kNad83CsrsPoint, and the latitude and longitude that European Petroleum Survey Group (EPSG)
	// operation 1946 returns for kNad83CsrsLatitudeAndLongitude with an ellipsoidal height of 0 metres.
	std::vector<double> const kNad83CsrsLatitudeAndLongitude{ 45.0, -75.0 };
	std::vector<double> const kExpectedWgs84LatitudeAndLongitudeForZeroEllipsoidalHeight{ 45.000009002478, -75.000001910865 };
}

// NAD83(CSRS) three-dimensional to WGS 84 three-dimensional converts the ellipsoidal height with EPSG operation 1946.
TEST(Geographic3DDatumShift, Nad83CsrsToWgs84ConvertsTheEllipsoidalHeight)
{
	ExpectPointCoordinatesWithinTolerances(kWgs84Point, TransformPointWithEpsgOperation1946(4955, 4979, kNad83CsrsPoint), kLatitudeLongitudeHeightTolerances);
}

// WGS 84 three-dimensional to NAD83(CSRS) three-dimensional applies the inverse of EPSG operation 1946.
TEST(Geographic3DDatumShift, Wgs84ToNad83CsrsAppliesTheInverseOperation)
{
	ExpectPointCoordinatesWithinTolerances(kNad83CsrsPoint, TransformPointWithEpsgOperation1946(4979, 4955, kWgs84Point), kLatitudeLongitudeHeightTolerances);
}

// Between a three-dimensional and a two-dimensional geographic coordinate reference system, the transformation receives
// and returns latitude and longitude, and the operation takes an ellipsoidal height of 0 metres: North American Datum
// 1983 (Canadian Spatial Reference System) three-dimensional to World Geodetic System 1984 two-dimensional.
TEST(Geographic3DDatumShift, Nad83Csrs3DToWgs84TwoDimensionalTransformsLatitudeAndLongitude)
{
	ExpectPointCoordinatesWithinTolerances(kExpectedWgs84LatitudeAndLongitudeForZeroEllipsoidalHeight, TransformPointWithEpsgOperation1946(4955, 4326, kNad83CsrsLatitudeAndLongitude), kLatitudeLongitudeHeightTolerances);
}

// The transformation from two-dimensional North American Datum 1983 (Canadian Spatial Reference System) to
// three-dimensional World Geodetic System 1984 receives and returns latitude and longitude.
TEST(Geographic3DDatumShift, Nad83Csrs2DToWgs84ThreeDimensionalTransformsLatitudeAndLongitude)
{
	ExpectPointCoordinatesWithinTolerances(kExpectedWgs84LatitudeAndLongitudeForZeroEllipsoidalHeight, TransformPointWithEpsgOperation1946(4617, 4979, kNad83CsrsLatitudeAndLongitude), kLatitudeLongitudeHeightTolerances);
}

// The transformation from the coordinate reference system with European Petroleum Survey Group (EPSG) code 4979 to a
// compound coordinate reference system of European Datum 1950 (ED50), defined in Well-Known Text version 1 with
// TOWGS84 parameters for the datum transformation from ED50 to World Geodetic System 1984 (WGS 84) and with longitude
// and latitude axes, and Earth Gravitational Model 1996 (EGM96) height (EPSG code 5773) applies the datum shift to the
// horizontal coordinates and uses the EGM96 geoid grid to convert the ellipsoidal height to EGM96 height. The test is
// skipped when the grid WW15MGH.GRD is missing.
TEST(Geographic3DDatumShift, Wgs84ToCompoundOfEd50AndEgm96HeightTransformsTheHorizontalCoordinatesWithTowgs84)
{
	auto const compoundCoordinateSystem = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(COMPD_CS["ED50 + EGM96 height",GEOGCS["ED50",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]],VERT_CS["EGM96 height",VERT_DATUM["EGM96 geoid",2005,AUTHORITY["EPSG","5171"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Up",UP],AUTHORITY["EPSG","5773"]]])WKT");
	try
	{
		auto const point = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4979), compoundCoordinateSystem)
			->GetMathTransform()->Transform({ 40.0, -3.0, 100.0 });
		ExpectPointCoordinatesWithinTolerances({ -2.998800648958, 40.001171406487, 47.613998 }, point, kLatitudeLongitudeHeightTolerances);
	}
	catch (GridFileNotFoundException const& exception)
	{
		GTEST_SKIP() << exception.what();
	}
}