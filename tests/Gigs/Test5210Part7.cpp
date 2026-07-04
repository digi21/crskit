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
            TestDirectTransformVertical(d, 72, 100, 1E-6);
            TestDirectTransformVertical(d, 66.67, 94.67, 1E-6);
            TestDirectTransformVertical(d, 17, 45, 1E-6);
            TestDirectTransformVertical(d, 0, 28, 1E-6);
            TestDirectTransformVertical(d, -28, 0, 1E-6);
            TestDirectTransformVertical(d, -36, -8, 1E-6);
            TestDirectTransformVertical(d, -44.3, -16.3, 1E-6);
            TestDirectTransformVertical(d, -210, -182, 1E-6);
        	}

	const char* kMt_Test5210_part_7_MathTransform = R"WKT(PARAM_MT["vertical_offset",PARAMETER["vertical_offset",28]])WKT";
}

TEST(Test5210Part7, Test5210_part_7_Epsg)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(5705);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(5611);
	ExecuteTests(BuildVerticalTransform(source, target));
}

TEST(Test5210Part7, Test5210_part_7_WktEpsg)
{
	const char* kSource = R"WKT(VERT_CS["Baltic height",VERT_DATUM["Baltic Sea",2005,AUTHORITY["EPSG","5105"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["H", Up],AUTHORITY["EPSG","5705"]])WKT";
	const char* kTarget = R"WKT(VERT_CS["Caspian height",VERT_DATUM["Caspian Sea",2005,AUTHORITY["EPSG","5106"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["H", Up],AUTHORITY["EPSG","5611"]])WKT";
	auto const source = std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(kSource));
	auto const target = std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(kTarget));
	ASSERT_TRUE(source); ASSERT_TRUE(target);
	ExecuteTests(BuildVerticalTransform(source, target));
}

TEST(Test5210Part7, Test5210_part_7_MathTransform)
{
	ExecuteTests(GetMathTransformFactory()->CreateFromWkt(kMt_Test5210_part_7_MathTransform));
}
