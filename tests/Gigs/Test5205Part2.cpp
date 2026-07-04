#include "../TestBase.h"
#include "../GigsFactory.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

// GIGS 5205 (part 2): Molodensky-Badekas 10-parametros, EPSG Coordinate Operation
// Method 1039 (geog3D domain). Same transformation 61003 as part 1, but
// entre geog3DCRS C (64021, Amersfoort) y geog3DCRS A (64002, WGS 84).
// Expected values from the official file GIGS_tfm_5205_MolBad_output_part2.

namespace
{
	const char* kMtD_Test5205_part_2_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6377397.155],PARAMETER["semi_minor",6356078.962818189]],PARAM_MT["molodensky_badekas",PARAMETER["x_axis_translation",593.0297],PARAMETER["y_axis_translation",26.0038],PARAMETER["z_axis_translation",478.7534],PARAMETER["x_axis_rotation",0.000113027777777778],PARAMETER["y_axis_rotation",-9.74166666666667e-05],PARAMETER["z_axis_rotation",0.000519527777777778],PARAMETER["scale_difference",4.0812],PARAMETER["ordinate_1_of_evaluation_point",3903453.1482],PARAMETER["ordinate_2_of_evaluation_point",368135.3134],PARAMETER["ordinate_3_of_evaluation_point",5012970.3051]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]]])WKT";

	const char* kMtI_Test5205_part_2_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],INVERSE_MT[PARAM_MT["molodensky_badekas",PARAMETER["x_axis_translation",593.0297],PARAMETER["y_axis_translation",26.0038],PARAMETER["z_axis_translation",478.7534],PARAMETER["x_axis_rotation",0.000113027777777778],PARAMETER["y_axis_rotation",-9.74166666666667e-05],PARAMETER["z_axis_rotation",0.000519527777777778],PARAMETER["scale_difference",4.0812],PARAMETER["ordinate_1_of_evaluation_point",3903453.1482],PARAMETER["ordinate_2_of_evaluation_point",368135.3134],PARAMETER["ordinate_3_of_evaluation_point",5012970.3051]]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6377397.155],PARAMETER["semi_minor",6356078.962818189]],PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]]])WKT";

	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
		constexpr double sigmaAng = 3E-7;
		constexpr double sigmaLin = 1E-2;

		// FORWARD: C (Amersfoort) -> A (WGS 84). Directa.
		TestTransform3D(d, 60, 120, 900, 60.00441729, 119.9900169, 519.593, sigmaAng, sigmaLin);
		TestTransform3D(d, 60, 120, 0, 60.00441792, 119.9900156, -380.411, sigmaAng, sigmaLin);
		TestTransform3D(d, 0, 0, 0, 0.0041133, -0.00007025, -148.564, sigmaAng, sigmaLin);
		TestTransform3D(d, 0, 0, -3000, 0.00411529, -0.00007004, -3148.576, sigmaAng, sigmaLin);
		TestTransform3D(d, 0, 0, -10000, 0.00411996, -0.00006955, -10148.604, sigmaAng, sigmaLin);
		TestTransform3D(d, -60, -120, 0, -60.00081456, -119.9922412, -1230.32, sigmaAng, sigmaLin);
		TestTransform3D(d, -60, -120, -900, -60.0008147, -119.99224, -2130.324, sigmaAng, sigmaLin);
		TestTransform3D(d, 70, -180, 0, 70.00666951, 179.9978617, -410.882, sigmaAng, sigmaLin);
		TestTransform3D(d, 25, -90, 0, 25.00457271, -89.99487454, -550.442, sigmaAng, sigmaLin);
		TestTransform3D(d, -37.6532236, 143.9279419, 1836.947, -37.65282045, 143.9233072, 525.013, sigmaAng, sigmaLin);
		TestTransform3D(d, -37.6532236, 143.9279419, 0, -37.65282034, 143.923306, -1311.941, sigmaAng, sigmaLin);
		TestTransform3D(d, -37.6532236, 143.9279419, -3000, -37.65282015, 143.923304, -4311.954, sigmaAng, sigmaLin);
		TestTransform3D(d, -37.6532236, 143.9279419, -10000, -37.65281972, 143.9232994, -11311.982, sigmaAng, sigmaLin);
		TestTransform3D(d, -70, 180, 0, -70.00362084, 179.9984789, -1286.11, sigmaAng, sigmaLin);

		// REVERSE: A (WGS 84) -> C (Amersfoort). Inversa.
		TestTransform3D(i, 80, 150, 1214.137, 79.99494735, 150.0181405, 1485.658, sigmaAng, sigmaLin);
		TestTransform3D(i, 80, 150, 0, 79.99494639, 150.0181437, 271.526, sigmaAng, sigmaLin);
		TestTransform3D(i, 30, 60, 189.569, 29.99746111, 60.00534993, 371.808, sigmaAng, sigmaLin);
		TestTransform3D(i, 30, 60, 0, 29.99746103, 60.00535007, 182.24, sigmaAng, sigmaLin);
		TestTransform3D(i, -30, -60, 0, -30.0042486, -60.00473241, 722.882, sigmaAng, sigmaLin);
		TestTransform3D(i, -30, -60, -526.476, -30.00424895, -60.00473285, 196.408, sigmaAng, sigmaLin);
		TestTransform3D(i, -30, -60, -571.476, -30.00424898, -60.00473289, 151.409, sigmaAng, sigmaLin);
		TestTransform3D(i, -80, -150, 0, -79.99612229, -150.0120919, 1197.381, sigmaAng, sigmaLin);
		TestTransform3D(i, -80, -150, -971.255, -79.99612168, -150.0120938, 226.13, sigmaAng, sigmaLin);
		TestTransform3D(i, -80, -150, -3316.255, -79.9961202, -150.0120983, -2118.86, sigmaAng, sigmaLin);
		TestTransform3D(i, 50, -135, 0, 49.99357566, -135.0045485, 597.819, sigmaAng, sigmaLin);
		TestTransform3D(i, 0, 0, 0, -0.00411349, 0.00007032, 148.593, sigmaAng, sigmaLin);
		TestTransform3D(i, -50, 135, 0, -49.99958243, 135.0064107, 1265.808, sigmaAng, sigmaLin);
	}
}

TEST(Test5205Part2, DISABLED_Test5205_part_2_Epsg)
{
	GTEST_SKIP() << "Not applicable: EPSG does not define a direct Molodensky-Badekas operation (method 1039, geog3D domain) towards WGS 84; EPSG dataset limit.";
}

TEST(Test5205Part2, Test5205_part_2_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5205_part_2_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5205_part_2_MathTransform);
	ExecuteTests(d, i);
}
