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
            TestDirectTransformVertical(d, -100, -72, 1E-6);
            TestDirectTransformVertical(d, -94.67, -66.67, 1E-6);
            TestDirectTransformVertical(d, -45, -17, 1E-6);
            TestDirectTransformVertical(d, -28, 0, 1E-6);
            TestDirectTransformVertical(d, 0, 28, 1E-6);
            TestDirectTransformVertical(d, 8, 36, 1E-6);
            TestDirectTransformVertical(d, 16.3, 44.3, 1E-6);
            TestDirectTransformVertical(d, 182, 210, 1E-6);
        	}

	const char* kMt_Test5210_part_6_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["vertical_offset",PARAMETER["vertical_offset",-28]]]])WKT";
}

TEST(Test5210Part6, Test5210_part_6_Epsg)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(5706);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(5612);
	ExecuteTests(BuildVerticalTransform(source, target));
}

TEST(Test5210Part6, Test5210_part_6_WktEpsg)
{
	const char* kSource = R"WKT(VERT_CS["Caspian depth",VERT_DATUM["Caspian Sea",2005,AUTHORITY["EPSG","5106"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["D", Down],AUTHORITY["EPSG","5706"]])WKT";
	const char* kTarget = R"WKT(VERT_CS["Baltic depth",VERT_DATUM["Baltic Sea",2005,AUTHORITY["EPSG","5105"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["D", Down],AUTHORITY["EPSG","5612"]])WKT";
	auto const source = std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(kSource));
	auto const target = std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(kTarget));
	ASSERT_TRUE(source); ASSERT_TRUE(target);
	ExecuteTests(BuildVerticalTransform(source, target));
}

TEST(Test5210Part6, Test5210_part_6_MathTransform)
{
	ExecuteTests(GetMathTransformFactory()->CreateFromWkt(kMt_Test5210_part_6_MathTransform));
}
