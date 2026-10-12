// Contract of the IUtilities functions that build a WKT or a name from an EPSG code: an empty string only
// for a code that does not exist or whose kind cannot be created; any other failure throws. Not part of
// the GIGS suite (no "Test5xxx" prefix).

#include <string>

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	// The projected CRS EPSG 25830 (ETRS89 / UTM zone 30N), as WKT.
	auto ProjectedWkt() -> std::string
	{
		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(25830)->GetWkt();
	}
}

class UtilitiesEmptyForCode : public ::testing::TestWithParam<int> {};

TEST_P(UtilitiesEmptyForCode, EveryCodeFunctionReturnsEmpty)
{
	auto const u = GetUtilities();
	auto const code = GetParam();
	EXPECT_EQ("", u->GetWktFromEpsgCode(code));
	EXPECT_EQ("", u->GetWktFromGeographicEpsgCode(code, true));
	EXPECT_EQ("", u->GetWktFromGeographicEpsgCode(code, false));
	EXPECT_EQ("", u->GetWktFromEpsgCodeWithEastNorthAxes(code));
	EXPECT_EQ("", u->GetCoordinateReferenceSystemName(code));
}

// 999999 and -1 do not exist; 0 is Local; 7415 (Amersfoort / RD New + NAP height) is compound.
INSTANTIATE_TEST_SUITE_P(MissingOrUncreatable, UtilitiesEmptyForCode, ::testing::Values(999999, 0, -1, 7415));

TEST(UtilitiesContract, ValidCodesReturnTheirWkt)
{
	auto const u = GetUtilities();
	EXPECT_TRUE(u->GetWktFromEpsgCode(25830).starts_with("PROJCS[")) << u->GetWktFromEpsgCode(25830);
	EXPECT_TRUE(u->GetWktFromEpsgCode(5782).starts_with("VERT_CS[")) << u->GetWktFromEpsgCode(5782);
	EXPECT_TRUE(u->GetWktFromEpsgCode(4326).starts_with("GEOGCS[")) << u->GetWktFromEpsgCode(4326);
	EXPECT_TRUE(u->GetWktFromEpsgCode(4979).starts_with("GEOGCS[")) << u->GetWktFromEpsgCode(4979);

	for (auto const code : { 25830, 5782, 4326, 4979 })
	{
		EXPECT_FALSE(u->GetWktFromEpsgCodeWithEastNorthAxes(code).empty()) << code;
		EXPECT_FALSE(u->GetCoordinateReferenceSystemName(code).empty()) << code;
	}
	EXPECT_EQ("ETRS89 / UTM zone 30N", u->GetCoordinateReferenceSystemName(25830));
}

// The geographic variant also returns empty for an existing code that is not geographic.
TEST(UtilitiesContract, GeographicVariantOnlyAcceptsGeographicCodes)
{
	auto const u = GetUtilities();
	EXPECT_TRUE(u->GetWktFromGeographicEpsgCode(4326, true).starts_with("GEOGCS["));
	EXPECT_TRUE(u->GetWktFromGeographicEpsgCode(4979, false).starts_with("GEOGCS["));
	EXPECT_EQ("", u->GetWktFromGeographicEpsgCode(25830, true));
	EXPECT_EQ("", u->GetWktFromGeographicEpsgCode(5782, true));
}

TEST(UtilitiesContract, GeographicOfProjectedWkt)
{
	auto const u = GetUtilities();
	EXPECT_TRUE(u->GetWktFromGeographic3DSystemForProjected(ProjectedWkt()).starts_with("GEOGCS["));
	EXPECT_EQ("", u->GetWktFromGeographic3DSystemForProjected(u->GetWktFromEpsgCode(4326)));
	EXPECT_THROW((void)u->GetWktFromGeographic3DSystemForProjected("this is not valid wkt"), WktParseException);
}

TEST(UtilitiesContract, CrsCodeWithEastNorthAxes)
{
	auto const u = GetUtilities();
	EXPECT_TRUE(u->GetWktFromCrsCodeWithEastNorthAxes("EPSG:25830").starts_with("PROJCS[\"ETRS89 / UTM zone 30N\""));
	EXPECT_TRUE(u->GetWktFromCrsCodeWithEastNorthAxes("CRS:84").starts_with("GEOGCS[\"WGS 84\""));
	EXPECT_EQ(u->GetWktFromEpsgCodeWithEastNorthAxes(4326), u->GetWktFromCrsCodeWithEastNorthAxes("CRS:84"));
}

// A code that gives no WKT, or a prefix that is not EPSG/CRS, throws "not recognized".
TEST(UtilitiesContract, CrsCodeWithEastNorthAxesRejectsUnknownCodes)
{
	auto const u = GetUtilities();
	for (auto const* code : { "EPSG:999999", "EPSG:7415", "EPSG:", "EPSG:abc", "CRS:99", "FOO:1", "" })
		EXPECT_THROW((void)u->GetWktFromCrsCodeWithEastNorthAxes(code), std::runtime_error) << code;
}

TEST(UtilitiesContract, EastNorthOfMalformedWktThrows)
{
	auto const u = GetUtilities();
	EXPECT_TRUE(u->GetWktWithEastNorthAxes(ProjectedWkt()).starts_with("PROJCS["));
	EXPECT_THROW((void)u->GetWktWithEastNorthAxes("this is not valid wkt"), WktParseException);
}
