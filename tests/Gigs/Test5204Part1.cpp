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
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(120, 0, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(59, 59, 53.064, CardinalPoint::N), Sexa2DecimalDegrees(120, 0, 07.096, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(0, 0, 02.917, CardinalPoint::S), Sexa2DecimalDegrees(0, 0, 03.533, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(120, 0, 00.000, CardinalPoint::W), Sexa2DecimalDegrees(59, 59, 59.406, CardinalPoint::S), Sexa2DecimalDegrees(120, 0, 05.425, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(180, 0, 00.000, CardinalPoint::W), Sexa2DecimalDegrees(69, 59, 53.261, CardinalPoint::N), Sexa2DecimalDegrees(179, 59, 57.832, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(25, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(90, 0, 00.000, CardinalPoint::W), Sexa2DecimalDegrees(24, 59, 55.056, CardinalPoint::N), Sexa2DecimalDegrees(90, 0, 02.181, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(37, 39, 11.605, CardinalPoint::S), Sexa2DecimalDegrees(143, 55, 40.591, CardinalPoint::E), Sexa2DecimalDegrees(37, 39, 09.255, CardinalPoint::S), Sexa2DecimalDegrees(143, 55, 42.860, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(70, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(180, 0, 00.000, CardinalPoint::E), Sexa2DecimalDegrees(69, 59, 56.460, CardinalPoint::S), Sexa2DecimalDegrees(179, 59, 55.992, CardinalPoint::E), 1E-3);

            TestDirectTransform(d, Sexa2DecimalDegrees(80, 0, 05.586, CardinalPoint::N), Sexa2DecimalDegrees(149, 59, 53.714, CardinalPoint::E), Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(150, 0, 00.000, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(30, 0, 04.827, CardinalPoint::N), Sexa2DecimalDegrees(59, 59, 53.599, CardinalPoint::E), Sexa2DecimalDegrees(30, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(29, 59, 57.981, CardinalPoint::S), Sexa2DecimalDegrees(59, 59, 59.688, CardinalPoint::W), Sexa2DecimalDegrees(30, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(80, 0, 01.977, CardinalPoint::S), Sexa2DecimalDegrees(149, 59, 43.158, CardinalPoint::W), Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(150, 0, 00.000, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(50, 0, 06.588, CardinalPoint::N), Sexa2DecimalDegrees(134, 59, 56.090, CardinalPoint::W), Sexa2DecimalDegrees(50, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(135, 0, 00.000, CardinalPoint::W), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(0, 0, 02.917, CardinalPoint::N), Sexa2DecimalDegrees(0, 0, 03.533, CardinalPoint::W), Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::E), 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(50, 0, 03.461, CardinalPoint::S), Sexa2DecimalDegrees(134, 59, 56.885, CardinalPoint::E), Sexa2DecimalDegrees(50, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(135, 0, 00.000, CardinalPoint::E), 1E-3);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(80, 0, 05.586, CardinalPoint::N), Sexa2DecimalDegrees(149, 59, 53.714, CardinalPoint::E), 2E-4);
        	}

	const char* kMtD_Test5204_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine", PARAMETER["num_row",3], PARAMETER["num_col", 3],PARAMETER["elt_0_0", 0],PARAMETER["elt_0_1", 1],PARAMETER["elt_0_2", 0],PARAMETER["elt_1_0", 1],PARAMETER["elt_1_1", 0],PARAMETER["elt_1_2", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 1]],PARAM_MT["Affine", PARAMETER["num_row",4], PARAMETER["num_col", 3],PARAMETER["elt_0_0", 1],PARAMETER["elt_0_1", 0],PARAMETER["elt_0_2", 0],PARAMETER["elt_1_0", 0],PARAMETER["elt_1_1", 1],PARAMETER["elt_1_2", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 0],PARAMETER["elt_3_0", 0],PARAMETER["elt_3_1", 0],PARAMETER["elt_3_2", 1]],PARAM_MT["Ellipsoid_To_Geocentric", PARAMETER["semi_major",6378388], PARAMETER["semi_minor", 6356911.946127947]],PARAM_MT["Affine", PARAMETER["num_row",4], PARAMETER["num_col", 4],PARAMETER["elt_0_0", 0.9999987253],PARAMETER["elt_0_1", -8.93122624875125e-006],PARAMETER["elt_0_2", -2.215595698447141e-006],PARAMETER["elt_0_3", -106.8686],PARAMETER["elt_1_0", 8.93122624875125e-006],PARAMETER["elt_1_1", 0.9999987253],PARAMETER["elt_1_2", -1.631880770453627e-006],PARAMETER["elt_1_3", 52.2978],PARAMETER["elt_2_0", 2.215595698447141e-006],PARAMETER["elt_2_1", 1.631880770453627e-006],PARAMETER["elt_2_2", 0.9999987253],PARAMETER["elt_2_3", -103.7239],PARAMETER["elt_3_0", 0],PARAMETER["elt_3_1", 0],PARAMETER["elt_3_2", 0],PARAMETER["elt_3_3", 1]],PARAM_MT["Geocentric_To_Ellipsoid", PARAMETER["semi_major",6378137], PARAMETER["semi_minor", 6356752.314245179]],PARAM_MT["Affine", PARAMETER["num_row",3], PARAMETER["num_col", 4],PARAMETER["elt_0_0", 1],PARAMETER["elt_0_1", 0],PARAMETER["elt_0_2", 0],PARAMETER["elt_0_3", 0],PARAMETER["elt_1_0", 0],PARAMETER["elt_1_1", 1],PARAMETER["elt_1_2", 0],PARAMETER["elt_1_3", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 0],PARAMETER["elt_2_3", 1]],PARAM_MT["Affine", PARAMETER["num_row",3], PARAMETER["num_col", 3],PARAMETER["elt_0_0", 0],PARAMETER["elt_0_1", 1],PARAMETER["elt_0_2", 0],PARAMETER["elt_1_0", 1],PARAMETER["elt_1_1", 0],PARAMETER["elt_1_2", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 1]]])WKT";
	const char* kMtI_Test5204_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine", PARAMETER["num_row",3], PARAMETER["num_col", 3],PARAMETER["elt_0_0", 0],PARAMETER["elt_0_1", 1],PARAMETER["elt_0_2", 0],PARAMETER["elt_1_0", 1],PARAMETER["elt_1_1", 0],PARAMETER["elt_1_2", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 1]],PARAM_MT["Affine", PARAMETER["num_row",4], PARAMETER["num_col", 3],PARAMETER["elt_0_0", 1],PARAMETER["elt_0_1", 0],PARAMETER["elt_0_2", 0],PARAMETER["elt_1_0", 0],PARAMETER["elt_1_1", 1],PARAMETER["elt_1_2", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 0],PARAMETER["elt_3_0", 0],PARAMETER["elt_3_1", 0],PARAMETER["elt_3_2", 1]],PARAM_MT["Ellipsoid_To_Geocentric", PARAMETER["semi_major",6378137], PARAMETER["semi_minor", 6356752.314245179]],PARAM_MT["Affine", PARAMETER["num_row",4], PARAMETER["num_col", 4],PARAMETER["elt_0_0", 1.000001274616949],PARAMETER["elt_0_1", 8.931245401681099e-006],PARAMETER["elt_0_2", 2.215615921456212e-006],PARAMETER["elt_0_3", 106.8684989443674],PARAMETER["elt_1_0", -8.931252632884781e-006],PARAMETER["elt_1_1", 1.000001274619195],PARAMETER["elt_1_2", 1.631865142573763e-006],PARAMETER["elt_1_3", -52.29865186682799],PARAMETER["elt_2_0", -2.215586771952001e-006],PARAMETER["elt_2_1", -1.631904718698019e-006],PARAMETER["elt_2_2", 1.000001274694053],PARAMETER["elt_2_3", 103.7238807846086],PARAMETER["elt_3_0", 0],PARAMETER["elt_3_1", 0],PARAMETER["elt_3_2", 0],PARAMETER["elt_3_3", 1]],PARAM_MT["Geocentric_To_Ellipsoid", PARAMETER["semi_major",6378388], PARAMETER["semi_minor", 6356911.946127947]],PARAM_MT["Affine", PARAMETER["num_row",3], PARAMETER["num_col", 4],PARAMETER["elt_0_0", 1],PARAMETER["elt_0_1", 0],PARAMETER["elt_0_2", 0],PARAMETER["elt_0_3", 0],PARAMETER["elt_1_0", 0],PARAMETER["elt_1_1", 1],PARAMETER["elt_1_2", 0],PARAMETER["elt_1_3", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 0],PARAMETER["elt_2_3", 1]],PARAM_MT["Affine", PARAMETER["num_row",3], PARAMETER["num_col", 3],PARAMETER["elt_0_0", 0],PARAMETER["elt_0_1", 1],PARAMETER["elt_0_2", 0],PARAMETER["elt_1_0", 1],PARAMETER["elt_1_1", 0],PARAMETER["elt_1_2", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 1]]])WKT";
}

TEST(Test5204Part1, Test5204_part_1_Epsg)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4313);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326);
	auto const [d, i] = BuildDirectInverseWithOperation(source, target, 15929);
	ExecuteTests(d, i);
}

TEST(Test5204Part1, Test5204_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5204_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5204_part_1_MathTransform);
	ExecuteTests(d, i);
}
