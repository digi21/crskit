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

// NAD83(CSRS) three-dimensional to WGS 84 two-dimensional uses the ellipsoidal height of the input point and returns the
// latitude and the longitude.
TEST(Geographic3DDatumShift, Nad83Csrs3DToWgs84TwoDimensionalReturnsLatitudeAndLongitude)
{
	ExpectPointCoordinatesWithinTolerances({ kWgs84Point[0], kWgs84Point[1] }, TransformPointWithEpsgOperation1946(4955, 4326, kNad83CsrsPoint), kLatitudeLongitudeHeightTolerances);
}

// NAD83(CSRS) two-dimensional to WGS 84 three-dimensional uses an ellipsoidal height of 0 metres because the input point
// contains only latitude and longitude.
TEST(Geographic3DDatumShift, Nad83Csrs2DToWgs84ThreeDimensionalUsesZeroMetresAsInputEllipsoidalHeight)
{
	ExpectPointCoordinatesWithinTolerances({ 45.000009002478, -75.000001910865, -1.124226 }, TransformPointWithEpsgOperation1946(4617, 4979, { kNad83CsrsPoint[0], kNad83CsrsPoint[1] }), kLatitudeLongitudeHeightTolerances);
}
