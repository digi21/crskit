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
            TestDirectTransformVertical(d, 72, -100, 1E-6);
            TestDirectTransformVertical(d, 66.67, -94.67, 1E-6);
            TestDirectTransformVertical(d, 17, -45, 1E-6);
            TestDirectTransformVertical(d, 0, -28, 1E-6);
            TestDirectTransformVertical(d, -28, 0, 1E-6);
            TestDirectTransformVertical(d, -36, 8, 1E-6);
            TestDirectTransformVertical(d, -44.3, 16.3, 1E-6);
            TestDirectTransformVertical(d, -210, 182, 1E-6);
        	}

	const char* kMt_Test5210_part_8_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine", PARAMETER["num_row",2], PARAMETER["num_col", 2],PARAMETER["elt_0_0", -1],PARAMETER["elt_0_1", 0],PARAMETER["elt_1_0", 0],PARAMETER["elt_1_1", -1]],PARAM_MT["vertical_offset",PARAMETER["vertical_offset",-28]]])WKT";
}

TEST(Test5210Part8, DISABLED_Test5210_part_8_Epsg) { GTEST_SKIP() << "Not applicable: EPSG only defines a DEPRECATED vertical offset operation between these vertical datums; not used (not a pending failure, it is an EPSG dataset limit)."; }

TEST(Test5210Part8, DISABLED_Test5210_part_8_WktEpsg) { GTEST_SKIP() << "Not applicable: EPSG only defines a DEPRECATED vertical offset operation between these vertical datums; not used (not a pending failure, it is an EPSG dataset limit)."; }

TEST(Test5210Part8, Test5210_part_8_MathTransform)
{
	ExecuteTests(GetMathTransformFactory()->CreateFromWkt(kMt_Test5210_part_8_MathTransform));
}
