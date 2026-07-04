// Ported from TestDigi21OpenGIS/Test5210Part1.cs (GIGS 5210 series, part 1).
// 1D vertical test: height <-> depth over the same vertical datum W
// (only the Up/Down axis direction changes -> the lib resolves it with an Affine -1).

#include "../TestBase.h"
#include "../GigsFactory.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	void ExecuteTests(MathTransform const& d)
	{
		TestDirectTransformVertical(d, 100, -100, 1E-6);
		TestDirectTransformVertical(d, 94.67, -94.67, 1E-6);
		TestDirectTransformVertical(d, 45, -45, 1E-6);
		TestDirectTransformVertical(d, 28, -28, 1E-6);
		TestDirectTransformVertical(d, 0, 0, 1E-6);
		TestDirectTransformVertical(d, -8, 8, 1E-6);
		TestDirectTransformVertical(d, -16.3, 16.3, 1E-6);
		TestDirectTransformVertical(d, -182, 182, 1E-6);
	}

	const char* kWktA = R"WKT(VERT_CS["GIGS vertCRS W1 height",VERT_DATUM["GIGS vertical datum W",2005,AUTHORITY["GIGS","66603"]],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["H", Up],AUTHORITY["GIGS","64507"]])WKT";
	const char* kWktB = R"WKT(VERT_CS["GIGS vertCRS W1 depth",VERT_DATUM["GIGS vertical datum W",2005,AUTHORITY["GIGS","66603"]],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["D", Down],AUTHORITY["GIGS","64508"]])WKT";
	const char* kWktEpsgA = R"WKT(VERT_CS["Caspian height",VERT_DATUM["Caspian Sea",2005,AUTHORITY["EPSG","5106"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["H", Up],AUTHORITY["EPSG","5611"]])WKT";
	const char* kWktEpsgB = R"WKT(VERT_CS["Caspian depth",VERT_DATUM["Caspian Sea",2005,AUTHORITY["EPSG","5106"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["D", Down],AUTHORITY["EPSG","5706"]])WKT";
	const char* kMt = R"WKT(CONCAT_MT[PARAM_MT["Affine", PARAMETER["num_row",2], PARAMETER["num_col", 2],PARAMETER["elt_0_0", -1],PARAMETER["elt_0_1", 0],PARAMETER["elt_1_0", 0],PARAMETER["elt_1_1", -1]]])WKT";

	auto VertFromWkt(const char* wkt) -> std::shared_ptr<VerticalCoordinateSystem>
	{
		return std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt));
	}
}

TEST(Test5210Part1, Test5210_part_1)
{
	Gigs::GigsFactory gigs;
	auto const vertA = gigs.CreateVerticalCoordinateSystem("64507");
	auto const vertB = gigs.CreateVerticalCoordinateSystem("64508");
	ExecuteTests(BuildVerticalTransform(vertA, vertB));
}

TEST(Test5210Part1, Test5210_part_1_Wkt)
{
	auto const vertA = VertFromWkt(kWktA);
	auto const vertB = VertFromWkt(kWktB);
	ASSERT_TRUE(vertA);
	ASSERT_TRUE(vertB);
	ExecuteTests(BuildVerticalTransform(vertA, vertB));
}

TEST(Test5210Part1, Test5210_part_1_Epsg)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(5611);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(5706);
	ExecuteTests(BuildVerticalTransform(source, target));
}

TEST(Test5210Part1, Test5210_part_1_WktEpsg)
{
	auto const vertA = VertFromWkt(kWktEpsgA);
	auto const vertB = VertFromWkt(kWktEpsgB);
	ASSERT_TRUE(vertA);
	ASSERT_TRUE(vertB);
	ExecuteTests(BuildVerticalTransform(vertA, vertB));
}

TEST(Test5210Part1, Test5210_part_1_MathTransform)
{
	ExecuteTests(GetMathTransformFactory()->CreateFromWkt(kMt));
}
