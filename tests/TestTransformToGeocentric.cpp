// Utilities::CreateTransformToGeocentric builds the chain from a horizontal and a vertical coordinate reference system
// (CRS) to geocentric coordinates (direct) and the chain from geocentric coordinates to coordinates in the horizontal and
// the vertical CRS (inverse). With a geoid operation, which receives three
// coordinates and returns the height, each chain takes the height conversion that the factory builds in its direction.
// AlicanteHeightRoundTripsThroughGeocentricCoordinates needs the geoid grid EGM08_REDNAP.txt in the data directory, and
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

// EGM2008 height (EPSG 3855) has two geoid operations from WGS 84 3D (EPSG 3858 and 3859). CreateTransformToGeocentric
// builds both chains with the operation that the user chooses and asks for it once.
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

	// The ellipsoidal height of the direct chain equals, within 1 mm, the height of the point transformed with EPSG operation
	// 3858 from WGS 84 + EGM2008 height to WGS 84 3D (EPSG 4979), so the geoid is interpolated at the latitude and longitude
	// of the point.
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

// A vertical CRS without code has no operation in the catalogue: the dialog creates once a vertical offset from the
// geographic CRS to the vertical CRS, the direct chain applies its inverse and the inverse chain applies it.
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