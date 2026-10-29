// StaticCoordinateTransformationFactory::CreateFromCoordinateSystems between a geographic and a geocentric coordinate
// reference system (CRS) whose datums differ. A geographic-to-geocentric transformation shifts the datum to the geographic
// CRS of the geocentric datum before converting to geocentric coordinates; a geocentric-to-geographic transformation
// converts to geographic coordinates before shifting the datum. The expected values were computed outside crskit with the
// formulas of the European Petroleum Survey Group (EPSG) for the geocentric conversion, the geocentric translations and
// the coordinate frame rotation.

#include "TestBase.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	// European Datum 1950 (ED50) with the International 1924 ellipsoid and TOWGS84[-87,-98,-121] for the datum
	// transformation from ED50 to World Geodetic System 1984 (WGS 84).
	constexpr char const* kEd50WithTowgs84Wkt = R"WKT(GEOGCS["ED50 3D",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AXIS["Ellipsoidal height",UP]])WKT";
	constexpr char const* kEd50GeocentricWithTowgs84Wkt = R"WKT(GEOCCS["ED50 geocentric",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["metre",1])WKT";

	// Latitude and longitude in degrees, ellipsoidal height in metres.
	std::vector<double> const kEd50Point{ 40.0, -3.0, 100.0 };
	std::vector<double> const kWgs84PointOfEd50Point{ 39.998828592800, -3.001199318117, 172.922441 };
	// Geocentric X, Y and Z in metres.
	std::vector<double> const kEd50GeocentricPointOfEd50Point{ 4886299.729795, -256080.117750, 4078118.084125 };
	std::vector<double> const kWgs84GeocentricPointOfEd50Point{ 4886212.729795, -256178.117750, 4077997.084125 };

	// Latitude 45 degrees, longitude -75 degrees and ellipsoidal height 100 metres in North American Datum 1983 (Canadian
	// Spatial Reference System), abbreviated NAD83(CSRS), and the WGS 84 geocentric coordinates that EPSG operation 1946
	// gives for that point.
	std::vector<double> const kNad83CsrsPoint{ 45.0, -75.0, 100.0 };
	std::vector<double> const kWgs84GeocentricPointOfNad83CsrsPoint{ 1169256.324328, -4363724.591573, 4487419.032019 };

	// Latitude 40 degrees, longitude -3 degrees and ellipsoidal height 100 metres in ED50 (EPSG code 4230) give, with EPSG
	// operation 1133 (geocentric translations -87, -98 and -121 metres) applied to latitude and longitude for an
	// ellipsoidal height of 0 metres, these WGS 84 geocentric coordinates for an ellipsoidal height of 100 metres.
	std::vector<double> const kWgs84GeocentricPointOfEd50PointWithUnchangedHeight{ 4886156.944854, -256175.194620, 4077950.210059 };

	constexpr int kNad83CsrsToWgs84OperationCode = 1946;
	constexpr int kEd50ToWgs84OperationCode = 1133;

	std::vector<double> const kLatitudeLongitudeHeightTolerances{ 1e-9, 1e-9, 1e-3 };
	std::vector<double> const kGeocentricTolerances{ 1e-3, 1e-3, 1e-3 };
	// The inverse of EPSG operation 1133 assumes an ellipsoidal height of 0 metres in WGS 84. For kEd50Point, applying EPSG
	// operation 1133 followed by the inverse operation changes latitude and longitude by about 1.3e-8 degrees.
	std::vector<double> const kLatitudeLongitudeHeightTolerancesAfterTwoDimensionalDatumShift{ 1e-6, 1e-6, 1e-3 };

	auto Transform(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, std::vector<double> const& point) -> std::vector<double>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)->GetMathTransform()->Transform(point);
	}

	// Returns the transformation from the coordinate reference system identified by EPSG code sourceCode to the coordinate
	// reference system identified by EPSG code targetCode; the transformation uses the catalogue operation operationCode
	// when the catalogue has several operations between the two datums.
	auto CreateTransformationWithOperation(int sourceCode, int targetCode, int operationCode) -> std::shared_ptr<IMathTransform>
	{
		CoordinateTransformationOptions options;
		options.selectOperation = [operationCode](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return operationCode; };
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(sourceCode),
			GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(targetCode),
			options)->GetMathTransform();
	}

	auto ExpectPointCoordinatesWithinTolerances(std::vector<double> const& expectedPoint, std::vector<double> const& point, std::vector<double> const& tolerances) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerances[coordinateIndex]) << coordinateIndex;
	}

	auto CreateCrs(int code) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code);
	}
}

// The transformation from ED50 with TOWGS84 to WGS 84 geocentric (EPSG code 4978) applies the TOWGS84 translations.
TEST(GeographicToGeocentricDatumShift, Ed50WithTowgs84ToWgs84GeocentricAppliesTowgs84)
{
	ExpectPointCoordinatesWithinTolerances(kWgs84GeocentricPointOfEd50Point, Transform(GetCoordinateSystemFactory()->CreateFromWkt(kEd50WithTowgs84Wkt), CreateCrs(4978), kEd50Point), kGeocentricTolerances);
}

// The transformation from EPSG 4978 to ED50 with TOWGS84 applies the inverse of the TOWGS84 translations.
TEST(GeographicToGeocentricDatumShift, Wgs84GeocentricToEd50WithTowgs84AppliesInverseTowgs84)
{
	ExpectPointCoordinatesWithinTolerances(kEd50Point, Transform(CreateCrs(4978), GetCoordinateSystemFactory()->CreateFromWkt(kEd50WithTowgs84Wkt), kWgs84GeocentricPointOfEd50Point), kLatitudeLongitudeHeightTolerances);
}

// The transformation from WGS 84 3D (EPSG code 4979) to an ED50 geocentric CRS with TOWGS84 and without authority code
// applies the inverse of the TOWGS84 translations, and the inverse transformation applies the translations.
TEST(GeographicToGeocentricDatumShift, Wgs84ToEd50GeocentricWithTowgs84AppliesInverseTowgs84)
{
	auto const transformation = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCrs(4979), GetCoordinateSystemFactory()->CreateFromWkt(kEd50GeocentricWithTowgs84Wkt))->GetMathTransform();
	ExpectPointCoordinatesWithinTolerances(kEd50GeocentricPointOfEd50Point, transformation->Transform(kWgs84PointOfEd50Point), kGeocentricTolerances);
	ExpectPointCoordinatesWithinTolerances(kWgs84PointOfEd50Point, transformation->GetInverse()->Transform(kEd50GeocentricPointOfEd50Point), kLatitudeLongitudeHeightTolerances);
}

// The transformation from NAD83(CSRS) 3D (EPSG code 4955) to EPSG 4978 applies the catalogue operation between NAD83(CSRS)
// and WGS 84, EPSG 1946, to latitude, longitude and ellipsoidal height.
TEST(GeographicToGeocentricDatumShift, Nad83Csrs3DToWgs84GeocentricAppliesCatalogueOperation)
{
	ExpectPointCoordinatesWithinTolerances(kWgs84GeocentricPointOfNad83CsrsPoint, CreateTransformationWithOperation(4955, 4978, kNad83CsrsToWgs84OperationCode)->Transform(kNad83CsrsPoint), kGeocentricTolerances);
}

// From NAD83(CSRS) 2D (EPSG code 4617), the transformation receives the ellipsoidal height as third coordinate and shifts
// the datum with the geographic 3D CRS of the catalogue, EPSG 4955, so the geocentric coordinates equal those obtained by
// transforming from NAD83(CSRS) 3D (EPSG 4955) to WGS 84 geocentric (EPSG 4978) with EPSG operation 1946.
TEST(GeographicToGeocentricDatumShift, Nad83Csrs2DToWgs84GeocentricUsesGeographic3DCrsOfCatalogue)
{
	auto const transformation = CreateTransformationWithOperation(4617, 4978, kNad83CsrsToWgs84OperationCode);
	ExpectPointCoordinatesWithinTolerances(kWgs84GeocentricPointOfNad83CsrsPoint, transformation->Transform(kNad83CsrsPoint), kGeocentricTolerances);
	ExpectPointCoordinatesWithinTolerances(kNad83CsrsPoint, transformation->GetInverse()->Transform(kWgs84GeocentricPointOfNad83CsrsPoint), kLatitudeLongitudeHeightTolerances);
}

// The catalogue has no geographic 3D CRS for ED50. From ED50 2D (EPSG code 4230), the transformation applies EPSG operation
// 1133, a transformation between EPSG 4230 and WGS 84 (EPSG code 4326), to latitude and longitude, keeps the ellipsoidal
// height and converts the result to geocentric coordinates with the WGS 84 ellipsoid.
TEST(GeographicToGeocentricDatumShift, Ed50TwoDimensionalToWgs84GeocentricKeepsEllipsoidalHeight)
{
	auto const transformation = CreateTransformationWithOperation(4230, 4978, kEd50ToWgs84OperationCode);
	ExpectPointCoordinatesWithinTolerances(kWgs84GeocentricPointOfEd50PointWithUnchangedHeight, transformation->Transform(kEd50Point), kGeocentricTolerances);
	ExpectPointCoordinatesWithinTolerances(kEd50Point, transformation->GetInverse()->Transform(kWgs84GeocentricPointOfEd50PointWithUnchangedHeight), kLatitudeLongitudeHeightTolerancesAfterTwoDimensionalDatumShift);
}

// CreateFromCoordinateSystems calls selectOperation once from European Datum 1950 (ED50; code 4230 of the European
// Petroleum Survey Group (EPSG) catalogue) to EPSG 4978 and uses the selected operation for the transformation and for its inverse.
TEST(GeographicToGeocentricDatumShift, Ed50TwoDimensionalToWgs84GeocentricCallsSelectOperationOnce)
{
	auto selectOperationCallCount = 0;
	CoordinateTransformationOptions options;
	options.selectOperation = [&selectOperationCallCount](std::string const&, std::string const&, std::vector<CoordinateOperation> const&)
	{
		++selectOperationCallCount;
		return kEd50ToWgs84OperationCode;
	};
	auto const transformation = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCrs(4230), CreateCrs(4978), options)->GetMathTransform();
	EXPECT_EQ(1, selectOperationCallCount);
	ExpectPointCoordinatesWithinTolerances(kEd50Point, transformation->GetInverse()->Transform(kWgs84GeocentricPointOfEd50PointWithUnchangedHeight), kLatitudeLongitudeHeightTolerancesAfterTwoDimensionalDatumShift);
}

// World Geodetic System 1984 (WGS 84), realization G1762, geographic 3D (European Petroleum Survey Group (EPSG) code 7665)
// and WGS 84 geocentric (EPSG 4978) have different datums and the catalogue has no operation between EPSG 7665 and EPSG
// 4979, but the two datums have the WGS 84 ellipsoid and the Greenwich meridian, so the transformation converts with the
// WGS 84 ellipsoid.
TEST(GeographicToGeocentricDatumShift, Wgs84RealizationToWgs84GeocentricConvertsWithoutDatumShift)
{
	ExpectPointCoordinatesWithinTolerances({ 4886078.813811, -256068.540034, 4078049.850961 }, Transform(CreateCrs(7665), CreateCrs(4978), kEd50Point), kGeocentricTolerances);
}

// European Terrestrial Reference System 1989 (ETRS89) defined in Well-Known Text version 1 with the datum name D_ETRS_1989
// of the Environmental Systems Research Institute (ESRI) and ETRS89 geocentric (European Petroleum Survey Group (EPSG)
// code 4936) use the Geodetic Reference System 1980 (GRS 1980) ellipsoid and the Greenwich meridian, so the
// transformation converts with the GRS 1980 ellipsoid.
TEST(GeographicToGeocentricDatumShift, Etrs89WithEsriDatumNameToEtrs89GeocentricConvertsWithoutDatumShift)
{
	auto const etrs89 = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCS["GCS_ETRS_1989",DATUM["D_ETRS_1989",SPHEROID["GRS_1980",6378137,298.257222101]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AXIS["h",UP]])WKT");
	ExpectPointCoordinatesWithinTolerances({ 4886078.813845, -256068.540036, 4078049.850855 }, Transform(etrs89, CreateCrs(4936), kEd50Point), kGeocentricTolerances);
}

// CreateFromCoordinateSystems converts coordinates from a European Datum 1950 (ED50) geographic 3D coordinate reference
// system whose datum is named European_Datum_1950 to an ED50 geocentric system whose datum is named ED50. Neither datum
// defines TOWGS84 parameters for the transformation to World Geodetic System 1984. The datum names differ, but both
// systems use the International 1924 ellipsoid and the Greenwich prime meridian.
TEST(GeographicToGeocentricDatumShift, Ed50WithoutTowgs84ToGeocentricWithDatumNameEd50ConvertsWithoutDatumShift)
{
	auto const ed50 = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCS["ED50 3D",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AXIS["Ellipsoidal height",UP]])WKT");
	auto const ed50Geocentric = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOCCS["ED50 geocentric",DATUM["ED50",SPHEROID["International 1924",6378388,297]],PRIMEM["Greenwich",0],UNIT["metre",1])WKT");
	ExpectPointCoordinatesWithinTolerances(kEd50GeocentricPointOfEd50Point, Transform(ed50, ed50Geocentric, kEd50Point), kGeocentricTolerances);
}

// The catalogue has no operation between Nouvelle Triangulation Française (NTF) with the Paris prime meridian (European
// Petroleum Survey Group (EPSG) code 4807) and World Geodetic System 1984 (WGS 84; EPSG 4326), and the Clarke 1880 (Institut
// géographique national, IGN) ellipsoid of NTF differs from the WGS 84 ellipsoid, so CreateFromCoordinateSystems throws
// TransformationNotFoundException from EPSG 4807 to WGS 84 geocentric (EPSG 4978).
TEST(GeographicToGeocentricDatumShift, NtfParisToWgs84GeocentricThrowsTransformationNotFoundException)
{
	EXPECT_THROW(static_cast<void>(GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCrs(4807), CreateCrs(4978))), TransformationNotFoundException);
}
