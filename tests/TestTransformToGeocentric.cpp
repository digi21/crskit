// Utilities::CreateTransformToGeocentric builds the chain from a horizontal and a vertical coordinate reference system
// (CRS) to geocentric coordinates (direct) and the chain from geocentric coordinates to coordinates in the horizontal and
// the vertical CRS (inverse). With a geoid operation, which receives three
// coordinates and returns the height, each chain takes the height conversion that the factory builds in its direction.
// AlicanteHeightRoundTripsThroughGeocentricCoordinates and LatitudeFirstGeographicRoundTripsThroughGeocentricCoordinates need
// the geoid grid EGM08_REDNAP.txt in the data directory, and
// AsksOnceForTheGeoidOperationOfBothChains needs Und_min2.5x2.5_egm2008_isw=82_WGS84_TideFree (EPSG operation 3858);
// each test is skipped when a grid that it needs is missing.

#include "TestBase.h"
#include "../crskit/IUtilities.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	// Applies the first stepCount steps of the chain to the coordinates.
	auto ApplySteps(std::vector<std::shared_ptr<IMathTransform>> const& chain, std::vector<double> coordinates, std::size_t stepCount) -> std::vector<double>
	{
		for (std::size_t stepIndex = 0; stepIndex < stepCount; ++stepIndex)
			coordinates = chain[stepIndex]->Transform(coordinates);
		return coordinates;
	}

	auto ApplyChain(std::vector<std::shared_ptr<IMathTransform>> const& chain, std::vector<double> const& coordinates) -> std::vector<double>
	{
		return ApplySteps(chain, coordinates, chain.size());
	}

	auto CreateCatalogueCoordinateSystem(int code) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code); }

	// Tolerances of a round trip for each coordinate: 1 mm in metres, and 1e-8 degrees (about 1.1 mm of latitude).
	std::vector<double> const kEastingNorthingHeightTolerances{ 1e-3, 1e-3, 1e-3 };
	std::vector<double> const kLatitudeLongitudeHeightTolerances{ 1e-8, 1e-8, 1e-3 };

	// Applies the direct chain and then the inverse chain to the point, and compares each returned coordinate with the
	// starting coordinate within the tolerance of the same index.
	auto ExpectChainsReturnThePoint(std::vector<std::shared_ptr<IMathTransform>> const& direct, std::vector<std::shared_ptr<IMathTransform>> const& inverse, std::vector<double> const& point, std::vector<double> const& tolerances) -> void
	{
		auto const geocentricPoint = ApplyChain(direct, point);
		auto const returnedPoint = ApplyChain(inverse, geocentricPoint);
		ASSERT_EQ(point.size(), returnedPoint.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(point[coordinateIndex], returnedPoint[coordinateIndex], tolerances[coordinateIndex]) << "coordinate " << coordinateIndex;
	}

	auto CreateUnknownVerticalCoordinateSystem() { return GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetUnknownVerticalWkt()); }
}

// ETRS89 / UTM zone 30N + Alicante height (EPSG 25830 + 5782) with the geoid EGM08-REDNAP (EPSG operation 9410), at a point
// of Bronchales (Teruel). The direct chain converts the orthometric height to ellipsoidal height and the inverse chain
// returns the starting point within 1 mm.
TEST(TransformToGeocentric, AlicanteHeightRoundTripsThroughGeocentricCoordinates)
{
	auto const horizontalCoordinateSystem = CreateCatalogueCoordinateSystem(25830);
	auto const verticalCoordinateSystem = CreateCatalogueCoordinateSystem(5782);
	std::vector<std::shared_ptr<IMathTransform>> direct;
	std::vector<std::shared_ptr<IMathTransform>> inverse;
	auto verticalTransformationDialogCallCount = 0;
	auto operationSelectionCallCount = 0;

	try
	{
		GetUtilities()->CreateTransformToGeocentric(horizontalCoordinateSystem, verticalCoordinateSystem, direct, inverse,
			[&](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { ++verticalTransformationDialogCallCount; return nullptr; },
			[&](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { ++operationSelectionCallCount; return operations.front().Code; });
	}
	catch (GridFileNotFoundException const& exception)
	{
		GTEST_SKIP() << exception.what();
	}
	ASSERT_FALSE(direct.empty());
	ASSERT_FALSE(inverse.empty());
	EXPECT_EQ(0, verticalTransformationDialogCallCount);

	// Easting, northing (metres) and orthometric height (metres).
	std::vector<double> const projectedPoint{ 619173.470, 4485177.795, 1576.899 };

	// All the steps of the direct chain but the last give longitude and latitude in degrees and the ellipsoidal height.
	auto const geographicPoint = ApplySteps(direct, projectedPoint, direct.size() - 1);
	auto const expectedLatitudeAndLongitude = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		horizontalCoordinateSystem, CreateCatalogueCoordinateSystem(4258))->GetMathTransform()->Transform({ projectedPoint[0], projectedPoint[1] });
	EXPECT_NEAR(expectedLatitudeAndLongitude[1], geographicPoint[0], 1e-9);
	EXPECT_NEAR(expectedLatitudeAndLongitude[0], geographicPoint[1], 1e-9);

	// The ellipsoidal height equals, within 1 mm, the height of the point transformed from the compound CRS to ETRS89
	// 3D, and exceeds the orthometric height by more than 40 m and less than 60 m.
	auto const compoundCoordinateSystem = std::dynamic_pointer_cast<CoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(25830, 5782));
	auto const expectedGeographic3DPoint = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		compoundCoordinateSystem, CreateCatalogueCoordinateSystem(4937))->GetMathTransform()->Transform(projectedPoint);
	EXPECT_NEAR(expectedGeographic3DPoint[2], geographicPoint[2], 1e-3);
	EXPECT_GT(geographicPoint[2] - projectedPoint[2], 40.0);
	EXPECT_LT(geographicPoint[2] - projectedPoint[2], 60.0);

	auto const returnedPoint = ApplyChain(inverse, ApplyChain(direct, projectedPoint));
	EXPECT_NEAR(projectedPoint[0], returnedPoint[0], 1e-3);
	EXPECT_NEAR(projectedPoint[1], returnedPoint[1], 1e-3);
	EXPECT_NEAR(projectedPoint[2], returnedPoint[2], 1e-3);
}

// ETRS89 / UTM zone 30N with the unknown vertical CRS of GetUnknownVerticalWkt (LOCAL_CS). The test expects that
// CreateTransformToGeocentric does not call createVerticalSystemDialogCallback, that the height before the last step of the
// direct chain, GeographicToGeocentric, differs from the starting height by at most 1e-9 metres, and that the inverse chain
// returns the starting point within 1 mm.
TEST(TransformToGeocentric, UnknownVerticalRoundTripsThroughGeocentricCoordinates)
{
	std::vector<std::shared_ptr<IMathTransform>> direct;
	std::vector<std::shared_ptr<IMathTransform>> inverse;
	auto verticalTransformationDialogCallCount = 0;

	GetUtilities()->CreateTransformToGeocentric(CreateCatalogueCoordinateSystem(25830), CreateUnknownVerticalCoordinateSystem(), direct, inverse,
		[&](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { ++verticalTransformationDialogCallCount; return nullptr; },
		[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });
	EXPECT_EQ(0, verticalTransformationDialogCallCount);

	// Easting, northing and height (metres).
	std::vector<double> const projectedPoint{ 619173.470, 4485177.795, 1576.899 };
	EXPECT_NEAR(projectedPoint[2], ApplySteps(direct, projectedPoint, direct.size() - 1)[2], 1e-9);
	ExpectChainsReturnThePoint(direct, inverse, projectedPoint, kEastingNorthingHeightTolerances);
}

// ETRS89 3D (EPSG 4937, axes latitude, longitude and ellipsoidal height) as the horizontal CRS. The test expects that the
// inverse chain returns the starting point in the axis order of EPSG 4937.
TEST(TransformToGeocentric, Geographic3DRoundTripsThroughGeocentricCoordinates)
{
	std::vector<std::shared_ptr<IMathTransform>> direct;
	std::vector<std::shared_ptr<IMathTransform>> inverse;

	GetUtilities()->CreateTransformToGeocentric(CreateCatalogueCoordinateSystem(4937), CreateUnknownVerticalCoordinateSystem(), direct, inverse,
		[](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { return nullptr; },
		[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });

	// Latitude, longitude (degrees) and ellipsoidal height (metres) in Madrid.
	ExpectChainsReturnThePoint(direct, inverse, { 40.4, -3.7, 700.0 }, kLatitudeLongitudeHeightTolerances);
}

// WGS 84 3D (EPSG 4979) split by Utilities::ExtractHorizontalVertical into its geographic 2D CRS and EPSG 4979 as the
// vertical CRS. The test expects that the inverse chain returns the starting point in the axis order of EPSG 4979.
TEST(TransformToGeocentric, Geographic3DSplitIntoHorizontalAndVerticalRoundTripsThroughGeocentricCoordinates)
{
	auto const [horizontalCoordinateSystem, verticalCoordinateSystem] = GetUtilities()->ExtractHorizontalVertical(CreateCatalogueCoordinateSystem(4979));
	ASSERT_EQ(2, horizontalCoordinateSystem->GetDimension());
	std::vector<std::shared_ptr<IMathTransform>> direct;
	std::vector<std::shared_ptr<IMathTransform>> inverse;
	auto verticalTransformationDialogCallCount = 0;

	GetUtilities()->CreateTransformToGeocentric(horizontalCoordinateSystem, verticalCoordinateSystem, direct, inverse,
		[&](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { ++verticalTransformationDialogCallCount; return nullptr; },
		[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });
	EXPECT_EQ(0, verticalTransformationDialogCallCount);

	// Latitude, longitude (degrees) and ellipsoidal height (metres) in Madrid.
	ExpectChainsReturnThePoint(direct, inverse, { 40.4, -3.7, 700.0 }, kLatitudeLongitudeHeightTolerances);
}

// ETRS89 (EPSG 4258, axes latitude and longitude) + Alicante height (EPSG 5782) with the geoid EGM08-REDNAP (EPSG operation
// 9410). The test expects that the point before the last step of the direct chain, GeographicToGeocentric, has the
// longitude before the latitude, and that the inverse chain returns the starting point with the latitude before the
// longitude.
TEST(TransformToGeocentric, LatitudeFirstGeographicRoundTripsThroughGeocentricCoordinates)
{
	std::vector<std::shared_ptr<IMathTransform>> direct;
	std::vector<std::shared_ptr<IMathTransform>> inverse;

	try
	{
		GetUtilities()->CreateTransformToGeocentric(CreateCatalogueCoordinateSystem(4258), CreateCatalogueCoordinateSystem(5782), direct, inverse,
			[](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { return nullptr; },
			[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });
	}
	catch (GridFileNotFoundException const& exception)
	{
		GTEST_SKIP() << exception.what();
	}

	// Latitude, longitude (degrees) and Alicante height (metres) in Madrid.
	std::vector<double> const geographicPoint{ 40.4, -3.7, 650.0 };
	auto const longitudeLatitudeAndEllipsoidalHeight = ApplySteps(direct, geographicPoint, direct.size() - 1);
	EXPECT_NEAR(geographicPoint[1], longitudeLatitudeAndEllipsoidalHeight[0], 1e-9);
	EXPECT_NEAR(geographicPoint[0], longitudeLatitudeAndEllipsoidalHeight[1], 1e-9);
	EXPECT_GT(longitudeLatitudeAndEllipsoidalHeight[2] - geographicPoint[2], 40.0);
	ExpectChainsReturnThePoint(direct, inverse, geographicPoint, kLatitudeLongitudeHeightTolerances);
}

// EGM2008 height (EPSG 3855) has two geoid operations from WGS 84 3D (EPSG 3858 and 3859), offered in both directions.
// CreateTransformToGeocentric calls selectTransformationCallback once and reuses the selected operation code for both chains.
TEST(TransformToGeocentric, AsksOnceForTheGeoidOperationOfBothChains)
{
	std::vector<std::shared_ptr<IMathTransform>> direct;
	std::vector<std::shared_ptr<IMathTransform>> inverse;
	auto operationSelectionCallCount = 0;

	try
	{
		GetUtilities()->CreateTransformToGeocentric(CreateCatalogueCoordinateSystem(4326), CreateCatalogueCoordinateSystem(3855), direct, inverse,
			[](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { return nullptr; },
			[&](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations)
			{
				++operationSelectionCallCount;
				for (auto const& operation : operations)
					if (3858 == operation.Code)
						return operation.Code;
				return operations.front().Code;
			});
	}
	catch (GridFileNotFoundException const& exception)
	{
		GTEST_SKIP() << exception.what();
	}

	EXPECT_EQ(1, operationSelectionCallCount);

	// Latitude, longitude (degrees) and EGM2008 height (metres) in Madrid.
	std::vector<double> const geographicPoint{ 40.4, -3.7, 650.0 };

	// The ellipsoidal height after all the steps of the direct chain but the last, GeographicToGeocentric, equals, within 1 mm,
	// the height of the point transformed with EPSG operation 3858 from WGS 84 + EGM2008 height to WGS 84 3D (EPSG 4979).
	CoordinateTransformationOptions options;
	options.selectOperation = [](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return 3858; };
	auto const compoundCoordinateSystem = std::dynamic_pointer_cast<CoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(4326, 3855));
	auto const expectedGeographic3DPoint = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		compoundCoordinateSystem, CreateCatalogueCoordinateSystem(4979), options)->GetMathTransform()->Transform(geographicPoint);
	auto const longitudeLatitudeAndEllipsoidalHeight = ApplySteps(direct, geographicPoint, direct.size() - 1);
	EXPECT_NEAR(expectedGeographic3DPoint[2], longitudeLatitudeAndEllipsoidalHeight[2], 1e-3);
	auto const returnedPoint = ApplyChain(inverse, ApplyChain(direct, geographicPoint));
	EXPECT_NEAR(geographicPoint[0], returnedPoint[0], 1e-9);
	EXPECT_NEAR(geographicPoint[1], returnedPoint[1], 1e-9);
	EXPECT_NEAR(geographicPoint[2], returnedPoint[2], 1e-3);
}

// The catalogue has no operation for the vertical CRS without code of this test: the dialog callback returns verticalOffset
// once, the direct chain applies verticalOffset->GetInverse() and the inverse chain applies verticalOffset.
TEST(TransformToGeocentric, UsesTheVerticalOffsetOfTheDialogWhenTheCatalogueHasNoOperation)
{
	auto const verticalCoordinateSystem = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(VERTCRS["Local height without code",VDATUM["Local vertical datum"],CS[vertical,1],AXIS["gravity-related height (H)",up],LENGTHUNIT["metre",1]])WKT");
	auto const verticalOffset = GetMathTransformFactory()->CreateParameterizedTransform("vertical_offset", { Parameter{ "vertical_offset", 12.5 } });
	std::vector<std::shared_ptr<IMathTransform>> direct;
	std::vector<std::shared_ptr<IMathTransform>> inverse;
	auto verticalTransformationDialogCallCount = 0;

	GetUtilities()->CreateTransformToGeocentric(CreateCatalogueCoordinateSystem(25830), verticalCoordinateSystem, direct, inverse,
		[&](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { ++verticalTransformationDialogCallCount; return verticalOffset; },
		[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });
	EXPECT_EQ(1, verticalTransformationDialogCallCount);

	// Easting, northing (metres) and height in the vertical CRS without code (metres).
	std::vector<double> const projectedPoint{ 619173.470, 4485177.795, 1576.899 };
	auto const geographicPoint = ApplySteps(direct, projectedPoint, direct.size() - 1);
	EXPECT_NEAR(verticalOffset->GetInverse()->Transform({ projectedPoint[2] })[0], geographicPoint[2], 1e-9);

	auto const returnedPoint = ApplyChain(inverse, ApplyChain(direct, projectedPoint));
	EXPECT_NEAR(projectedPoint[0], returnedPoint[0], 1e-3);
	EXPECT_NEAR(projectedPoint[1], returnedPoint[1], 1e-3);
	EXPECT_NEAR(projectedPoint[2], returnedPoint[2], 1e-3);
}