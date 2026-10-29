// The rectangle of the area of use of each candidate coordinate operation (CoordinateOperation::AreaOfUseBoundingBox),
// read from the extent of the European Petroleum Survey Group (EPSG) catalogue, and GeographicBoundingBox::Contains,
// including extents that cross the antimeridian.

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "TestBase.h"
#include "../crskit/SqliteProvider.h"

using namespace CrsKit;
using CrsKit::CoordinateTransformations::CoordinateOperation;
using CrsKit::Positioning::GeographicBoundingBox;

namespace
{
	auto GetEpsgDatabaseFilePath() -> std::filesystem::path
	{
		if (char const* configuredEpsgDatabaseFilePath = std::getenv("DIGI21_EPSG_SQLITE"))
			return configuredEpsgDatabaseFilePath;
		return "C:\\ProgramData\\Digi3D.NET\\OpenGis\\epsg-v10_066.sqlite";
	}

	auto FindOperation(std::vector<CoordinateOperation> const& operations, int code) -> CoordinateOperation const*
	{
		auto const matchingOperationIterator = std::ranges::find(operations, code, &CoordinateOperation::Code);
		return matchingOperationIterator == operations.end() ? nullptr : &*matchingOperationIterator;
	}

	constexpr auto kEd50CrsCode = 4230;
	constexpr auto kWgs84CrsCode = 4326;
	constexpr auto kNzgd2000CrsCode = 4167;
	constexpr auto kMadridLatitudeDegrees = 40.41;
	constexpr auto kMadridLongitudeDegrees = -3.71;
}

// Among the datum shifts from European Datum 1950 (ED50) to World Geodetic System 1984 (WGS 84), the area of use of EPSG 1311 ("Europe - common offshore") does not contain
// Madrid and the area of use of EPSG 1633 ("Spain - mainland except northwest") does.
TEST(AreaOfUseBoundingBox, Ed50ToWgs84OperationsHaveAreaOfUseBoundingBoxes)
{
	auto const operations = GetCurrentEnvironment()->Provider->OperationCodesToTransformSystemAtoB(kEd50CrsCode, kWgs84CrsCode);

	auto const* const offshore = FindOperation(operations, 1311);
	ASSERT_NE(nullptr, offshore);
	ASSERT_TRUE(offshore->AreaOfUseBoundingBox.has_value());
	EXPECT_EQ((GeographicBoundingBox{ .SouthBoundLatitudeDegrees = 47.42, .NorthBoundLatitudeDegrees = 63.89, .WestBoundLongitudeDegrees = -16.1, .EastBoundLongitudeDegrees = 10.86 }), *offshore->AreaOfUseBoundingBox);
	EXPECT_FALSE(offshore->AreaOfUseBoundingBox->Contains(kMadridLatitudeDegrees, kMadridLongitudeDegrees));

	auto const* const spain = FindOperation(operations, 1633);
	ASSERT_NE(nullptr, spain);
	ASSERT_TRUE(spain->AreaOfUseBoundingBox.has_value());
	EXPECT_FALSE(spain->AreaOfUseBoundingBox->CrossesAntimeridian());
	EXPECT_TRUE(spain->AreaOfUseBoundingBox->Contains(kMadridLatitudeDegrees, kMadridLongitudeDegrees));
}

// The area of use of EPSG 1565 ("NZGD2000 to WGS 84 (1)", New Zealand) spans from longitude 160.6 degrees to -171.2
// degrees across the antimeridian: it contains the longitudes on both sides of 180 degrees and excludes the longitudes
// strictly between -171.2 and 160.6 degrees.
TEST(AreaOfUseBoundingBox, ExtentCrossingTheAntimeridian)
{
	auto const operations = GetCurrentEnvironment()->Provider->OperationCodesToTransformSystemAtoB(kNzgd2000CrsCode, kWgs84CrsCode);

	auto const* const newZealand = FindOperation(operations, 1565);
	ASSERT_NE(nullptr, newZealand);
	ASSERT_TRUE(newZealand->AreaOfUseBoundingBox.has_value());

	auto const& box = *newZealand->AreaOfUseBoundingBox;
	EXPECT_DOUBLE_EQ(160.6, box.WestBoundLongitudeDegrees);
	EXPECT_DOUBLE_EQ(-171.2, box.EastBoundLongitudeDegrees);
	EXPECT_TRUE(box.CrossesAntimeridian());

	constexpr auto latitudeDegrees = -41.29;
	EXPECT_TRUE(box.Contains(latitudeDegrees, 174.78));          // Wellington
	EXPECT_TRUE(box.Contains(latitudeDegrees, 180.0));
	EXPECT_TRUE(box.Contains(latitudeDegrees, -180.0));
	EXPECT_TRUE(box.Contains(latitudeDegrees, -175.0));
	EXPECT_TRUE(box.Contains(latitudeDegrees, 174.78 + 360.0));
	EXPECT_TRUE(box.Contains(latitudeDegrees, -171.2));
	EXPECT_TRUE(box.Contains(latitudeDegrees, 160.6));
	EXPECT_FALSE(box.Contains(latitudeDegrees, -171.0));
	EXPECT_FALSE(box.Contains(latitudeDegrees, 160.5));
	EXPECT_FALSE(box.Contains(latitudeDegrees, 0.0));
	EXPECT_FALSE(box.Contains(0.0, 174.78));
}

// Contains includes the boundary, treats longitudes 180 degrees and -180 degrees as the same meridian and returns false
// when either coordinate is NaN (not a number).
TEST(AreaOfUseBoundingBox, ContainsIncludesTheBoundaryAndRejectsNaN)
{
	GeographicBoundingBox const box{ .SouthBoundLatitudeDegrees = -10.0, .NorthBoundLatitudeDegrees = 10.0, .WestBoundLongitudeDegrees = 170.0, .EastBoundLongitudeDegrees = 180.0 };
	EXPECT_FALSE(box.CrossesAntimeridian());
	EXPECT_TRUE(box.Contains(-10.0, 170.0));
	EXPECT_TRUE(box.Contains(10.0, 180.0));
	EXPECT_TRUE(box.Contains(0.0, -180.0));
	EXPECT_TRUE(box.Contains(0.0, -190.0));
	EXPECT_FALSE(box.Contains(10.000001, 175.0));
	EXPECT_FALSE(box.Contains(0.0, -179.0));

	auto const nan = std::numeric_limits<double>::quiet_NaN();
	EXPECT_FALSE(box.Contains(nan, 175.0));
	EXPECT_FALSE(box.Contains(0.0, nan));
	EXPECT_FALSE(box.Contains(0.0, std::numeric_limits<double>::infinity()));
}

// In a copy of the catalogue, the test deletes the epsg_usage row of EPSG 1311 and sets BBOX_WEST_BOUND_LON to NULL in the
// extent of EPSG 1633: 1311 has no area of use and no rectangle, and 1633 keeps the name of its area of use without a rectangle.
TEST(AreaOfUseBoundingBox, OperationWithoutExtentHasNoRectangle)
{
	auto const catalogueCopyFilePath = std::filesystem::temp_directory_path() / "crskit-area-of-use-without-extent.sqlite";
	std::filesystem::copy_file(GetEpsgDatabaseFilePath(), catalogueCopyFilePath, std::filesystem::copy_options::overwrite_existing);
	std::filesystem::permissions(catalogueCopyFilePath, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);

	{
		sqlite3* db{};
		ASSERT_EQ(SQLITE_OK, sqlite3_open_v2(catalogueCopyFilePath.string().c_str(), &db, SQLITE_OPEN_READWRITE, nullptr));
		auto const catalogueModificationResultCode = sqlite3_exec(db,
			"DELETE FROM epsg_usage WHERE object_table_name='epsg_coordoperation' AND object_code=1311;"
			"UPDATE epsg_extent SET bbox_west_bound_lon=NULL WHERE extent_code="
			"(SELECT extent_code FROM epsg_usage WHERE object_table_name='epsg_coordoperation' AND object_code=1633);",
			nullptr, nullptr, nullptr);
		sqlite3_close(db);
		ASSERT_EQ(SQLITE_OK, catalogueModificationResultCode);
	}

	{
		Epsg::SQliteProvider provider{ catalogueCopyFilePath.string() };
		auto const operations = provider.OperationCodesToTransformSystemAtoB(kEd50CrsCode, kWgs84CrsCode);

		auto const* const withoutUsage = FindOperation(operations, 1311);
		ASSERT_NE(nullptr, withoutUsage);
		EXPECT_EQ("", withoutUsage->AreaOfUse);
		EXPECT_FALSE(withoutUsage->AreaOfUseBoundingBox.has_value());

		auto const* const withoutWestBound = FindOperation(operations, 1633);
		ASSERT_NE(nullptr, withoutWestBound);
		EXPECT_EQ("Spain - mainland except northwest", withoutWestBound->AreaOfUse);
		EXPECT_FALSE(withoutWestBound->AreaOfUseBoundingBox.has_value());
	}

	std::error_code catalogueCopyRemovalError;
	std::filesystem::remove(catalogueCopyFilePath, catalogueCopyRemovalError);
}
