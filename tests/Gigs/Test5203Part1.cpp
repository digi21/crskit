#include "../TestBase.h"
#include "../GigsFactory.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(120, 0, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(60, 0, 20.495, CardinalPoint::N), Sexa2DecimalDegrees(119, 59, 39.692, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(0, 0, 17.400, CardinalPoint::N), Sexa2DecimalDegrees(0, 0, 03.206, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(120, 0, 00.000, CardinalPoint::W), Sexa2DecimalDegrees(59, 59, 56.665, CardinalPoint::S), Sexa2DecimalDegrees(119, 59, 30.669, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(180, 0, 00.000, CardinalPoint::W), Sexa2DecimalDegrees(70, 0, 21.402, CardinalPoint::N), Sexa2DecimalDegrees(179, 59, 46.945, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(25, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(90, 0, 00.000, CardinalPoint::W), Sexa2DecimalDegrees(25, 0, 16.050, CardinalPoint::N), Sexa2DecimalDegrees(89, 59, 43.121, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(37, 39, 11.605, CardinalPoint::S), Sexa2DecimalDegrees(143, 55, 40.591, CardinalPoint::E), Sexa2DecimalDegrees(37, 39, 08.471, CardinalPoint::S), Sexa2DecimalDegrees(143, 55, 34.853, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(180, 0, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(70, 0, 08.946, CardinalPoint::S), Sexa2DecimalDegrees(179, 59, 47.765, CardinalPoint::W), 1E-3);

            TestInverseTransform(i, Sexa2DecimalDegrees(79, 59, 41.544, CardinalPoint::N), Sexa2DecimalDegrees(150, 0, 20.429, CardinalPoint::E), Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(150, 0, 00.000, CardinalPoint::E), 1E-3);
            TestInverseTransform(i, Sexa2DecimalDegrees(29, 59, 44.404, CardinalPoint::N), Sexa2DecimalDegrees(60, 0, 16.084, CardinalPoint::E), Sexa2DecimalDegrees(30, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::E), 1E-3);
            TestInverseTransform(i, Sexa2DecimalDegrees(30, 0, 18.167, CardinalPoint::S), Sexa2DecimalDegrees(60, 0, 12.854, CardinalPoint::W), Sexa2DecimalDegrees(30, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::W), 1E-3);
            TestInverseTransform(i, Sexa2DecimalDegrees(79, 59, 52.013, CardinalPoint::S), Sexa2DecimalDegrees(150, 1, 00.952, CardinalPoint::W), Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(150, 0, 00.000, CardinalPoint::W), 1E-3);
            TestInverseTransform(i, Sexa2DecimalDegrees(49, 59, 40.513, CardinalPoint::N), Sexa2DecimalDegrees(135, 0, 21.468, CardinalPoint::W), Sexa2DecimalDegrees(50, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(135, 0, 00.000, CardinalPoint::W), 1E-3);
            TestInverseTransform(i, Sexa2DecimalDegrees(0, 0, 17.400, CardinalPoint::S), Sexa2DecimalDegrees(0, 0, 03.206, CardinalPoint::E), Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::E), 1E-3);
            TestInverseTransform(i, Sexa2DecimalDegrees(49, 59, 59.029, CardinalPoint::S), Sexa2DecimalDegrees(135, 0, 10.483, CardinalPoint::E), Sexa2DecimalDegrees(50, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(135, 0, 00.000, CardinalPoint::E), 1E-3);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(120, 0, 00.000, CardinalPoint::E), 2E-4);
        	}

	const char* kMtD_Test5203_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6377563.396],PARAMETER["semi_minor",6356256.909237285]],PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0.999979511],PARAMETER["elt_0_1",-4.082047556156235e-006],PARAMETER["elt_0_2",1.197465256972197e-006],PARAMETER["elt_0_3",446.448],PARAMETER["elt_1_0",4.082047556156235e-006],PARAMETER["elt_1_1",0.999979511],PARAMETER["elt_1_2",-7.272056216430349e-007],PARAMETER["elt_1_3",-125.157],PARAMETER["elt_2_0",-1.197465256972197e-006],PARAMETER["elt_2_1",7.272056216430349e-007],PARAMETER["elt_2_2",0.999979511],PARAMETER["elt_2_3",542.06],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_2_3",1]]])WKT";
	const char* kMtI_Test5203_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",1.00002048940171],PARAMETER["elt_0_1",4.082215706223008e-006],PARAMETER["elt_0_2",-1.197511359518915e-006],PARAMETER["elt_0_3",-446.4559874115357],PARAMETER["elt_1_0",-4.082213964509019e-006],PARAMETER["elt_1_1",1.000020489402615],PARAMETER["elt_1_2",7.272403103879139e-007],PARAMETER["elt_1_3",125.1609926805404],PARAMETER["elt_2_0",1.19751729685972e-006],PARAMETER["elt_2_1",-7.27230533566723e-007],PARAMETER["elt_2_2",1.000020489417845],PARAMETER["elt_2_3",-542.0717321410309],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6377563.396],PARAMETER["semi_minor",6356256.909237285]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_2_3",1]]])WKT";
}

TEST(Test5203Part1, Test5203_part_1_Epsg)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4277);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326);
	auto const [d, i] = BuildDirectInverseWithOperation(source, target, 1314);
	ExecuteTests(d, i);
}

TEST(Test5203Part1, Test5203_part_1_inverse)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4277);
	auto const [d, i] = BuildDirectInverseWithOperation(source, target, 1314);
	ExecuteTests(i, d);
}

TEST(Test5203Part1, Test5203_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5203_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5203_part_1_MathTransform);
	ExecuteTests(d, i);
}
