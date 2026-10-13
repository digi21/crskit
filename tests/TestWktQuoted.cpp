// Quoted names with commas and brackets, and WKT1 parameter names as GDAL writes them. Not part of the GIGS suite (no "Test5xxx" prefix).

#include <string>
#include <vector>

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
// GDAL writes standard_parallel_1 (OGC 01-009) where crskit registers standard_parallel1: the WKT1 GDAL writes for EPSG
// 2154 (RGF93 v1 / Lambert-93), without identifiers, projects like PROJ 9.8.1.
TEST(WktParameterNames, GdalLambertConformalConic2SP)
{
	auto const projected = std::dynamic_pointer_cast<CoordinateSystems::ProjectedCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(
		R"(PROJCS["RGF93 v1 / Lambert-93",GEOGCS["RGF93 v1",DATUM["Reseau_Geodesique_Francais_1993_v1",SPHEROID["GRS 1980",6378137,298.257222101]],)"
		R"(PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",46.5],)"
		R"(PARAMETER["central_meridian",3],PARAMETER["standard_parallel_1",49],PARAMETER["standard_parallel_2",44],PARAMETER["false_easting",700000],)"
		R"(PARAMETER["false_northing",6600000],UNIT["metre",1],AXIS["Easting",EAST],AXIS["Northing",NORTH]])"));
	ASSERT_NE(nullptr, projected);
	auto const geographic = std::dynamic_pointer_cast<CoordinateSystems::CoordinateSystem>(projected->GetGeographicCoordinateSystem());
	auto const transform = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geographic, projected)->GetMathTransform();
	auto const latitudeFirst = geographic->GetAxis(0).GetOrientation() == CoordinateSystems::AxisOrientationEnum::North;
	auto const p = transform->Transform(latitudeFirst ? std::vector{ 45.0, 5.0 } : std::vector{ 5.0, 45.0 });
	EXPECT_NEAR(857581.8993196681, p[0], 1e-3);
	EXPECT_NEAR(6435414.7478354, p[1], 1e-3);
}