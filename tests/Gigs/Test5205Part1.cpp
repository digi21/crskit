#include "../TestBase.h"
#include "../GigsFactory.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

// GIGS 5205 (part 1): Molodensky-Badekas 10-parameter (Coordinate convention
// Frame), EPSG Coordinate Operation Method 9636, geog2D domain.
// GIGS transformation 61003 (GIGS geogCRS C -> GIGS geogCRS A):
//   dX=593.0297  dY=26.0038  dZ=478.7534 metre
//   rX=0.4069  rY=-0.3507  rZ=1.8703 arc-second  (en degrees: /3600)
//   dS=4.0812 ppm
//   punto de evaluacion P = (3903453.1482, 368135.3134, 5012970.3051) metre
//   geogCRS C (64006) = Amersfoort (GIGS ellipsoid C, Bessel 1841)
//   geogCRS A (64003) = WGS 84      (GIGS ellipsoid A)
// Expected values taken from the official file GIGS_tfm_5205_MolBad_output_part1.
// Uses the native "molodensky_badekas" algorithm (registered as EPSG 9636).

namespace
{
	// The rotations are passed in DEGREES (arc-second / 3600), because the algorithm
	// converts them to radians with DEGREES_TO_RADIANS like the rest of the library.
	const char* kMtD_Test5205_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6377397.155],PARAMETER["semi_minor",6356078.962818189]],PARAM_MT["molodensky_badekas",PARAMETER["x_axis_translation",593.0297],PARAMETER["y_axis_translation",26.0038],PARAMETER["z_axis_translation",478.7534],PARAMETER["x_axis_rotation",0.000113027777777778],PARAMETER["y_axis_rotation",-9.74166666666667e-05],PARAMETER["z_axis_rotation",0.000519527777777778],PARAMETER["scale_difference",4.0812],PARAMETER["ordinate_1_of_evaluation_point",3903453.1482],PARAMETER["ordinate_2_of_evaluation_point",368135.3134],PARAMETER["ordinate_3_of_evaluation_point",5012970.3051]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_2_3",1]]])WKT";

	// Inverse A -> C: ellipsoids swapped and INVERSE_MT over the Molodensky-
	// Badekas (the algorithm inverts the 4x4 affine matrix when inverse=true).
	const char* kMtI_Test5205_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],INVERSE_MT[PARAM_MT["molodensky_badekas",PARAMETER["x_axis_translation",593.0297],PARAMETER["y_axis_translation",26.0038],PARAMETER["z_axis_translation",478.7534],PARAMETER["x_axis_rotation",0.000113027777777778],PARAMETER["y_axis_rotation",-9.74166666666667e-05],PARAMETER["z_axis_rotation",0.000519527777777778],PARAMETER["scale_difference",4.0812],PARAMETER["ordinate_1_of_evaluation_point",3903453.1482],PARAMETER["ordinate_2_of_evaluation_point",368135.3134],PARAMETER["ordinate_3_of_evaluation_point",5012970.3051]]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6377397.155],PARAMETER["semi_minor",6356078.962818189]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_2_3",1]]])WKT";

	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
		// Official geographic tolerance: 0.0000003 degrees.
		constexpr double sigma = 3E-7;

		// FORWARD: input en C (Amersfoort), esperado en A (WGS 84). Directa.
		TestDirectTransform(d, 60, 120, 60.00441792, 119.9900156, sigma);
		TestDirectTransform(d, 0, 0, 0.0041133, -0.00007025, sigma);
		TestDirectTransform(d, -60, -120, -60.00081456, -119.9922412, sigma);
		TestDirectTransform(d, 70, -180, 70.00666951, 179.9978617, sigma);
		TestDirectTransform(d, 25, -90, 25.00457271, -89.99487454, sigma);
		TestDirectTransform(d, -37.6532236, 143.9279419, -37.65282034, 143.923306, sigma);
		TestDirectTransform(d, -70, 180, -70.00362084, 179.9984789, sigma);

		// REVERSE: input en A (WGS 84), esperado en C (Amersfoort). Inversa.
		TestInverseTransform(i, 79.99494639, 150.0181437, 80, 150, sigma);
		TestInverseTransform(i, 29.99746103, 60.00535007, 30, 60, sigma);
		TestInverseTransform(i, -30.0042486, -60.00473241, -30, -60, sigma);
		TestInverseTransform(i, -79.99612229, -150.0120919, -80, -150, sigma);
		TestInverseTransform(i, 49.99357566, -135.0045485, 50, -135, sigma);
		TestInverseTransform(i, -0.00411349, 0.00007032, 0, 0, sigma);
		TestInverseTransform(i, -49.99958243, 135.0064107, -50, 135, sigma);
	}
}

TEST(Test5205Part1, DISABLED_Test5205_part_1_Epsg)
{
	GTEST_SKIP() << "Not applicable: EPSG does not define a direct Molodensky-Badekas operation (method 9636) Amersfoort (4289) -> WGS 84 (4326); EPSG dataset limit.";
}

TEST(Test5205Part1, Test5205_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5205_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5205_part_1_MathTransform);
	ExecuteTests(d, i);
}
