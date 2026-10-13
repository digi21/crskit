// Quoted names may hold commas and brackets. Not part of the GIGS suite (no "Test5xxx" prefix).

#include <string>

#include "TestBase.h"

using namespace CrsKit;

// EPSG 32661 is "WGS 84 / UPS North (N,E)": its own WKT1, without identifiers, parses back with that name.
TEST(WktQuoted, NameWithACommaParses)
{
	auto wkt = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(32661)->GetWkt();
	ASSERT_NE(std::string::npos, wkt.find("(N,E)"));
	auto const parsed = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
	EXPECT_EQ("WGS 84 / UPS North (N,E)", parsed->GetName());
}

TEST(WktQuoted, NameWithBracketsParses)
{
	auto const parsed = GetCoordinateSystemFactory()->CreateFromWkt(
		R"(GEOGCS["a [b], c",DATUM["d",SPHEROID["s",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]])");
	EXPECT_EQ("a [b], c", parsed->GetName());
}
// A quote that is not closed, or a single quote inside a name (WKT doubles it), is reported as such.
TEST(WktQuoted, UnclosedQuoteIsReported)
{
	for (auto const* wkt : {
		R"(GEOGCS["WGS 84,DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]])",
		R"(GEOGCS["a"b",DATUM["d",SPHEROID["s",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]])" })
	{
		try
		{
			(void)GetCoordinateSystemFactory()->CreateFromWkt(wkt);
			ADD_FAILURE() << "no exception: " << wkt;
		}
		catch (WktParseException const& e)
		{
			EXPECT_TRUE(std::string{ e.what() }.contains("not closed")) << e.what();
		}
	}
}