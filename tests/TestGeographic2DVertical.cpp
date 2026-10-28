// StaticCoordinateTransformationFactory::CreateFromCoordinateSystems between a geographic two-dimensional coordinate
// reference system (CRS) and a vertical CRS. Each transformation receives and returns three coordinates: the two
// horizontal coordinates in the axes and angular unit of the geographic CRS and a height. The transformation from the
// geographic CRS receives the ellipsoidal height in metres and returns the gravity-related height; the transformation
// from the vertical CRS receives the gravity-related height and returns the ellipsoidal height in metres. The tests
// compare the returned heights with those returned by the transformations between the vertical CRS and the geographic
// three-dimensional CRS World Geodetic System 1984 (WGS 84; European Petroleum Survey Group (EPSG) code 4979), whose
// coordinates are latitude and longitude in degrees and ellipsoidal height in metres. The WGS 84 tests also compare the
// horizontal coordinates with those of EPSG 4979; the test of Nouvelle Triangulation Francaise (NTF) with the Paris prime
// meridian checks that the horizontal coordinates keep their input values. The tests use the Earth Gravitational Model
// 1996 (EGM96) geoid (EPSG operation 10084, grid WW15MGH.GRD) and are skipped when the grid is missing.

#include "TestBase.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr int kEgm96OperationCode = 10084;
	constexpr int kEgm96HeightCode = 5773;
	constexpr double kParisPrimeMeridianLongitudeFromGreenwichInDegrees = 2.5969213 * 0.9;
	constexpr double kLatitudeInDegrees = 46.8;
	constexpr double kLongitudeFromGreenwichInDegrees = 2.78722917;

	auto CreateCatalogueCoordinateSystem(int epsgCode) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(epsgCode); }

	// Creates transformation options whose selectOperation callback returns the code of the EGM96 operation, 10084.
	auto CreateEgm96Options() -> CoordinateTransformationOptions
	{
		CoordinateTransformationOptions options;
		options.selectOperation = [](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return kEgm96OperationCode; };
		return options;
	}

	auto Transform(int sourceEpsgCode, int targetEpsgCode, std::vector<double> const& point) -> std::vector<double>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCatalogueCoordinateSystem(sourceEpsgCode), CreateCatalogueCoordinateSystem(targetEpsgCode), CreateEgm96Options())
			->GetMathTransform()->Transform(point);
	}

	auto ExpectPointCoordinatesWithinTolerances(std::vector<double> const& expectedPoint, std::vector<double> const& point, std::vector<double> const& tolerances) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerances[coordinateIndex]) << coordinateIndex;
	}

	// Tolerances for latitude and longitude in degrees or grads and for the height in metres.
	std::vector<double> const kLatitudeLongitudeHeightTolerances{ 1e-9, 1e-9, 1e-6 };
}

// WGS 84 two-dimensional (EPSG code 4326) to EGM96 height (EPSG code 5773) returns the latitude, the longitude and the
// gravity-related height that EPSG 4979 to EPSG 5773 returns for the same point.
TEST(Geographic2DVertical, Wgs84ToEgm96HeightMatchesTheGeographic3DTransformation)
{
	try
	{
		std::vector<double> const point{ kLatitudeInDegrees, kLongitudeFromGreenwichInDegrees, 100.0 };
		ExpectPointCoordinatesWithinTolerances(Transform(4979, kEgm96HeightCode, point), Transform(4326, kEgm96HeightCode, point), kLatitudeLongitudeHeightTolerances);
	}
	catch (GridFileNotFoundException const& exception)
	{
		GTEST_SKIP() << exception.what();
	}
}

// EGM96 height to WGS 84 two-dimensional returns the latitude, the longitude and the ellipsoidal height that EPSG 5773 to
// EPSG 4979 returns for the same point.
TEST(Geographic2DVertical, Egm96HeightToWgs84MatchesTheGeographic3DTransformation)
{
	try
	{
		std::vector<double> const point{ kLatitudeInDegrees, kLongitudeFromGreenwichInDegrees, 51.67 };
		ExpectPointCoordinatesWithinTolerances(Transform(kEgm96HeightCode, 4979, point), Transform(kEgm96HeightCode, 4326, point), kLatitudeLongitudeHeightTolerances);
	}
	catch (GridFileNotFoundException const& exception)
	{
		GTEST_SKIP() << exception.what();
	}
}

// Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian (EPSG code 4807; latitude and longitude in grads)
// to EGM96 height keeps the latitude and the longitude in grads and returns the gravity-related height that EPSG 4979 to
// EPSG 5773 returns for the same latitude and longitude from Greenwich.
TEST(Geographic2DVertical, NtfParisToEgm96HeightInterpolatesTheGeoidAtTheLongitudeFromGreenwich)
{
	try
	{
		auto const longitudeFromParisInDegrees = kLongitudeFromGreenwichInDegrees - kParisPrimeMeridianLongitudeFromGreenwichInDegrees;
		auto const egm96PointWithWgs84HorizontalCoordinates = Transform(4979, kEgm96HeightCode, { kLatitudeInDegrees, kLongitudeFromGreenwichInDegrees, 100.0 });
		auto const egm96PointWithNtfParisHorizontalCoordinates = Transform(4807, kEgm96HeightCode, { kLatitudeInDegrees / 0.9, longitudeFromParisInDegrees / 0.9, 100.0 });
		ExpectPointCoordinatesWithinTolerances({ kLatitudeInDegrees / 0.9, longitudeFromParisInDegrees / 0.9, egm96PointWithWgs84HorizontalCoordinates[2] }, egm96PointWithNtfParisHorizontalCoordinates, kLatitudeLongitudeHeightTolerances);
	}
	catch (GridFileNotFoundException const& exception)
	{
		GTEST_SKIP() << exception.what();
	}
}