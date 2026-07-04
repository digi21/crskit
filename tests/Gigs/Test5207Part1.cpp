#include "../TestBase.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
            TestDirectTransform(d, Sexa2DecimalDegrees(10, 3, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(115, 0, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(10, 2, 55.094, CardinalPoint::S), Sexa2DecimalDegrees(115, 0, 04.504, CardinalPoint::E), 1E-6);
            TestDirectTransform(d, Sexa2DecimalDegrees(12, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(115, 0, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(11, 59, 55.119, CardinalPoint::S), Sexa2DecimalDegrees(115, 0, 04.541, CardinalPoint::E), 1E-6);
            TestDirectTransform(d, Sexa2DecimalDegrees(9, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 24.000, CardinalPoint::E), Sexa2DecimalDegrees(8, 59, 54.756, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 28.078, CardinalPoint::E), 1E-6);
            TestDirectTransform(d, Sexa2DecimalDegrees(9, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 36.000, CardinalPoint::E), Sexa2DecimalDegrees(8, 59, 54.756, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 40.078, CardinalPoint::E), 1E-6);
            TestDirectTransform(d, Sexa2DecimalDegrees(27, 7, 35.315, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 19.431, CardinalPoint::E), Sexa2DecimalDegrees(27, 7, 30.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 24.000, CardinalPoint::E), 1E-6);
            TestDirectTransform(d, Sexa2DecimalDegrees(27, 7, 30.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(27, 7, 24.685, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 04.568, CardinalPoint::E), 1E-6);
            TestDirectTransform(d, Sexa2DecimalDegrees(27, 7, 30.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 18.000, CardinalPoint::E), Sexa2DecimalDegrees(27, 7, 24.684, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 22.568, CardinalPoint::E), 1E-6);
            TestDirectTransform(d, Sexa2DecimalDegrees(28, 3, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(137, 50, 50.000, CardinalPoint::E), Sexa2DecimalDegrees(28, 2, 54.712, CardinalPoint::S), Sexa2DecimalDegrees(137, 50, 54.620, CardinalPoint::E), 1E-6);
            TestDirectTransform(d, Sexa2DecimalDegrees(28, 3, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 50, 50.000, CardinalPoint::E), Sexa2DecimalDegrees(28, 2, 54.668, CardinalPoint::S), Sexa2DecimalDegrees(138, 50, 54.573, CardinalPoint::E), 1E-6);

            TestInverseTransform(i, Sexa2DecimalDegrees(11, 0, 04.894, CardinalPoint::S), Sexa2DecimalDegrees(114, 59, 55.479, CardinalPoint::E), Sexa2DecimalDegrees(11, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(115, 0, 00.000, CardinalPoint::E), 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(9, 0, 05.244, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 37.922, CardinalPoint::E), Sexa2DecimalDegrees(9, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 42.000, CardinalPoint::E), 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(9, 0, 05.244, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 55.922, CardinalPoint::E), Sexa2DecimalDegrees(9, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 00.000, CardinalPoint::E), 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(9, 0, 05.244, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 13.922, CardinalPoint::E), Sexa2DecimalDegrees(9, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 18.000, CardinalPoint::E), 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(27, 7, 30.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 42.000, CardinalPoint::E), Sexa2DecimalDegrees(27, 7, 24.685, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 46.568, CardinalPoint::E), 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(27, 7, 35.315, CardinalPoint::S), Sexa2DecimalDegrees(138, 2, 55.432, CardinalPoint::E), Sexa2DecimalDegrees(27, 7, 30.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 00.000, CardinalPoint::E), 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(27, 7, 35.316, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 31.432, CardinalPoint::E), Sexa2DecimalDegrees(27, 7, 30.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 3, 36.000, CardinalPoint::E), 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(28, 3, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(136, 50, 50.000, CardinalPoint::E), Sexa2DecimalDegrees(28, 2, 54.697, CardinalPoint::S), Sexa2DecimalDegrees(136, 50, 54.691, CardinalPoint::E), 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(28, 3, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(139, 50, 50.000, CardinalPoint::E), Sexa2DecimalDegrees(28, 2, 54.626, CardinalPoint::S), Sexa2DecimalDegrees(139, 50, 54.511, CardinalPoint::E), 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(12, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(115, 0, 00.000, CardinalPoint::E));
        	}

	const char* kMtD = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["NTv2",PARAMETER["Latitude_and_longitude_difference_file","A66 National (13.09.01).gsb"]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
	const char* kMtI = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],INVERSE_MT[PARAM_MT["NTv2",PARAMETER["Latitude_and_longitude_difference_file","A66 National (13.09.01).gsb"]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5207Part1, Test5207_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD);
	auto const i = mtf->CreateFromWkt(kMtI);
	ExecuteTests(d, i);
}

TEST(Test5207Part1, Test5207_part_1_MathTransform_OutOfRange_1)
{
	auto const d = GetMathTransformFactory()->CreateFromWkt(kMtD);
	EXPECT_THROW(TestDirectTransform(d, Sexa2DecimalDegrees(8, 0, 0.000, CardinalPoint::S), Sexa2DecimalDegrees(115, 0, 00.000, CardinalPoint::E), 0.0, 0.0, 1E-6), std::runtime_error);
}

TEST(Test5207Part1, Test5207_part_1_OutOfRange_MathTransform_2)
{
	auto const d = GetMathTransformFactory()->CreateFromWkt(kMtD);
	EXPECT_THROW(TestDirectTransform(d, Sexa2DecimalDegrees(10, 0, 0.000, CardinalPoint::S), Sexa2DecimalDegrees(115, 0, 00.000, CardinalPoint::E), 0.0, 0.0, 1E-6), std::runtime_error);
}

// Epsg variants (authority factory) - 2nd pass.
TEST(Test5207Part1, Test5207_part_1_Epsg)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4202);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4283);
	auto const [d, i] = BuildDirectInverseWithOperation(source, target, 1803);
	ExecuteTests(d, i);
}

TEST(Test5207Part1, Test5207_part_1_Epsg_Inverse)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4283);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4202);
	auto const [d, i] = BuildDirectInverseWithOperation(source, target, 1803);
	ExecuteTests(i, d);
}
