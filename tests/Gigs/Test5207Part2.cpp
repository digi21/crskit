#include "../TestBase.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(142, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(69, 59, 59.305, CardinalPoint::N), Sexa2DecimalDegrees(142, 0, 10.097, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(140, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(69, 59, 59.381, CardinalPoint::N), Sexa2DecimalDegrees(140, 0, 10.063, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(138, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(69, 59, 59.552, CardinalPoint::N), Sexa2DecimalDegrees(138, 0, 10.646, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 00.192, CardinalPoint::N), Sexa2DecimalDegrees(130, 0, 09.854, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(128, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 00.481, CardinalPoint::N), Sexa2DecimalDegrees(128, 0, 09.856, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(88, 0, 20.070, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 02.194, CardinalPoint::N), Sexa2DecimalDegrees(88, 0, 20.457, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(88, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 02.193, CardinalPoint::N), Sexa2DecimalDegrees(88, 0, 00.386, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(87, 39, 39.930, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 02.172, CardinalPoint::N), Sexa2DecimalDegrees(87, 39, 40.218, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(49, 59, 59.999, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(50, 0, 00.143, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 03.257, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(48, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(47, 59, 59.960, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 03.069, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(47, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), Sexa2DecimalDegrees(46, 59, 59.965, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 02.971, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(51, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), Sexa2DecimalDegrees(50, 59, 59.177, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 09.847, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(49, 59, 59.999, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), Sexa2DecimalDegrees(49, 59, 59.112, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 10.026, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(48, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), Sexa2DecimalDegrees(47, 59, 58.846, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 09.808, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(47, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), Sexa2DecimalDegrees(46, 59, 58.699, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 09.647, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(48, 7, 0, CardinalPoint::N), Sexa2DecimalDegrees(131, 27, 20.000, CardinalPoint::W), Sexa2DecimalDegrees(48, 6, 58.799, CardinalPoint::N), Sexa2DecimalDegrees(131, 27, 26.137, CardinalPoint::W), 2E-4);
            TestDirectTransform(d, Sexa2DecimalDegrees(48, 7, 0, CardinalPoint::N), Sexa2DecimalDegrees(128, 27, 20.000, CardinalPoint::W), Sexa2DecimalDegrees(48, 6, 59.014, CardinalPoint::N), Sexa2DecimalDegrees(128, 27, 25.513, CardinalPoint::W), 2E-4);

            TestInverseTransform(i, Sexa2DecimalDegrees(70, 0, 00.695, CardinalPoint::N), Sexa2DecimalDegrees(141, 59, 49.903, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(142, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(70, 0, 00.645, CardinalPoint::N), Sexa2DecimalDegrees(140, 59, 49.991, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(141, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(70, 0, 00.579, CardinalPoint::N), Sexa2DecimalDegrees(138, 59, 49.918, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(139, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(70, 0, 00.167, CardinalPoint::N), Sexa2DecimalDegrees(134, 59, 49.495, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(135, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(69, 59, 59.519, CardinalPoint::N), Sexa2DecimalDegrees(128, 0, 02.645, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(128, 0, 12.500, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(69, 59, 59.201, CardinalPoint::N), Sexa2DecimalDegrees(125, 59, 50.598, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(126, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(69, 59, 57.807, CardinalPoint::N), Sexa2DecimalDegrees(87, 59, 59.616, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(88, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(51, 1, 01.040, CardinalPoint::N), Sexa2DecimalDegrees(112, 9, 56.496, CardinalPoint::W), Sexa2DecimalDegrees(51, 1, 01.234, CardinalPoint::N), Sexa2DecimalDegrees(112, 9, 59.788, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(50, 59, 59.817, CardinalPoint::N), Sexa2DecimalDegrees(111, 59, 56.738, CardinalPoint::W), Sexa2DecimalDegrees(51, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(49, 59, 59.855, CardinalPoint::N), Sexa2DecimalDegrees(111, 59, 56.74350, CardinalPoint::W), Sexa2DecimalDegrees(49, 59, 59.999, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(49, 0, 00.023, CardinalPoint::N), Sexa2DecimalDegrees(111, 59, 56.823, CardinalPoint::W), Sexa2DecimalDegrees(49, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(47, 0, 00.035, CardinalPoint::N), Sexa2DecimalDegrees(111, 59, 57.029, CardinalPoint::W), Sexa2DecimalDegrees(47, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(51, 0, 00.823, CardinalPoint::N), Sexa2DecimalDegrees(130, 30, 57.930, CardinalPoint::W), Sexa2DecimalDegrees(51, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(50, 0, 00.886, CardinalPoint::N), Sexa2DecimalDegrees(130, 30, 57.750, CardinalPoint::W), Sexa2DecimalDegrees(49, 59, 59.999, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(49, 0, 01.017, CardinalPoint::N), Sexa2DecimalDegrees(130, 30, 57.842, CardinalPoint::W), Sexa2DecimalDegrees(49, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(47, 0, 01.300, CardinalPoint::N), Sexa2DecimalDegrees(130, 30, 58.129, CardinalPoint::W), Sexa2DecimalDegrees(47, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(48, 7, 01.295, CardinalPoint::N), Sexa2DecimalDegrees(132, 59, 53.539, CardinalPoint::W), Sexa2DecimalDegrees(48, 7, 0, CardinalPoint::N), Sexa2DecimalDegrees(133, 0, 0, CardinalPoint::W), 2E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(48, 7, 01.138, CardinalPoint::N), Sexa2DecimalDegrees(130, 30, 57.953, CardinalPoint::W), Sexa2DecimalDegrees(48, 7, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), 2E-4);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(70, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(140, 0, 0, CardinalPoint::W));
        	}

	const char* kMtD = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["NTv2",PARAMETER["Latitude_and_longitude_difference_file","NTv2_0.gsb"]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
	const char* kMtI = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],INVERSE_MT[PARAM_MT["NTv2",PARAMETER["Latitude_and_longitude_difference_file","NTv2_0.gsb"]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5207Part2, Test5207_part_2_Epsg)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4267);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4269);
	auto const [d, i] = BuildDirectInverseWithOperation(source, target, 1313);
	ExecuteTests(d, i);
}

TEST(Test5207Part2, Test5207_part_2_Epsg_Inverse)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4269);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4267);
	auto const [d, i] = BuildDirectInverseWithOperation(source, target, 1313);
	ExecuteTests(i, d);
}

TEST(Test5207Part2, Test5207_part_2_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD);
	auto const i = mtf->CreateFromWkt(kMtI);
	ExecuteTests(d, i);
}

TEST(Test5207Part2, Test5207_part_2_MathTransform_OutOfRange_1)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD);
	auto const i = mtf->CreateFromWkt(kMtI);
	EXPECT_THROW(TestDirectTransform(d, Sexa2DecimalDegrees(46, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), 0.0, 0.0, 2E-4), std::runtime_error);
}

TEST(Test5207Part2, Test5207_part_2_Math_Transform_OutOfRange_2)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD);
	auto const i = mtf->CreateFromWkt(kMtI);
	EXPECT_THROW(TestDirectTransform(d, Sexa2DecimalDegrees(45, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), 0.0, 0.0, 2E-4), std::runtime_error);
}

TEST(Test5207Part2, Test5207_part_2_MathTransform_OutOfRange_3)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD);
	auto const i = mtf->CreateFromWkt(kMtI);
	EXPECT_THROW(TestInverseTransform(i, 0.0, 0.0, Sexa2DecimalDegrees(46, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(112, 0, 0, CardinalPoint::W), 2E-4), std::runtime_error);
}

TEST(Test5207Part2, Test5207_part_2_MathTransform_OutOfRange_4)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD);
	auto const i = mtf->CreateFromWkt(kMtI);
	EXPECT_THROW(TestInverseTransform(i, 0.0, 0.0, Sexa2DecimalDegrees(46, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(130, 31, 03.888, CardinalPoint::W), 2E-4), std::runtime_error);
}
