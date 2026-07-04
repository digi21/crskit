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
            TestDirectTransformVertical(d, 100, -72, 1E-6);
            TestDirectTransformVertical(d, 94.67, -66.67, 1E-6);
            TestDirectTransformVertical(d, 45, -17, 1E-6);
            TestDirectTransformVertical(d, 28, 0, 1E-6);
            TestDirectTransformVertical(d, 0, 28, 1E-6);
            TestDirectTransformVertical(d, -8, 36, 1E-6);
            TestDirectTransformVertical(d, -16.3, 44.3, 1E-6);
            TestDirectTransformVertical(d, -182, 210, 1E-6);
        	}

	const char* kMt_Test5210_part_3_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["vertical_offset",PARAMETER["vertical_offset",28]]],PARAM_MT["Affine", PARAMETER["num_row",2], PARAMETER["num_col", 2],PARAMETER["elt_0_0", -1],PARAMETER["elt_0_1", -0],PARAMETER["elt_1_0", -0],PARAMETER["elt_1_1", -1]]])WKT";
}

TEST(Test5210Part3, DISABLED_Test5210_part_3_Epsg) { GTEST_SKIP() << "Not applicable: EPSG only defines a DEPRECATED vertical offset operation between these vertical datums; not used (not a pending failure, it is an EPSG dataset limit)."; }

TEST(Test5210Part3, DISABLED_Test5210_part_3_WktEpsg) { GTEST_SKIP() << "Not applicable: EPSG only defines a DEPRECATED vertical offset operation between these vertical datums; not used (not a pending failure, it is an EPSG dataset limit)."; }

TEST(Test5210Part3, Test5210_part_3_MathTransform)
{
	ExecuteTests(GetMathTransformFactory()->CreateFromWkt(kMt_Test5210_part_3_MathTransform));
}
