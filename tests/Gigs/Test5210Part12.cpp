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
            TestDirectTransformVertical(d, -72, 72, 1E-6);
            TestDirectTransformVertical(d, -66.67, 66.67, 1E-6);
            TestDirectTransformVertical(d, -17, 17, 1E-6);
            TestDirectTransformVertical(d, 0, 0, 1E-6);
            TestDirectTransformVertical(d, 28, -28, 1E-6);
            TestDirectTransformVertical(d, 36, -36, 1E-6);
            TestDirectTransformVertical(d, 44.3, -44.3, 1E-6);
            TestDirectTransformVertical(d, 210, -210, 1E-6);
        	}

	const char* kMt_Test5210_part_12_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine", PARAMETER["num_row",2], PARAMETER["num_col", 2],PARAMETER["elt_0_0", -1],PARAMETER["elt_0_1", 0],PARAMETER["elt_1_0", 0],PARAMETER["elt_1_1", -1]]])WKT";
}

TEST(Test5210Part12, Test5210_part_12_Epsg)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(5612);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(5705);
	ExecuteTests(BuildVerticalTransform(source, target));
}

TEST(Test5210Part12, Test5210_part_12_WktEpsg)
{
	const char* kSource = R"WKT(VERT_CS["Baltic depth",VERT_DATUM["Baltic Sea",2005,AUTHORITY["EPSG","5105"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["D", Down],AUTHORITY["EPSG","5612"]])WKT";
	const char* kTarget = R"WKT(VERT_CS["Baltic height",VERT_DATUM["Baltic Sea",2005,AUTHORITY["EPSG","5105"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["H", Up],AUTHORITY["EPSG","5705"]])WKT";
	auto const source = std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(kSource));
	auto const target = std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(kTarget));
	ASSERT_TRUE(source); ASSERT_TRUE(target);
	ExecuteTests(BuildVerticalTransform(source, target));
}

TEST(Test5210Part12, Test5210_part_12_MathTransform)
{
	ExecuteTests(GetMathTransformFactory()->CreateFromWkt(kMt_Test5210_part_12_MathTransform));
}
