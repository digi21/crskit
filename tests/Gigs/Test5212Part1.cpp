#include "../TestBase.h"
#include "../GigsFactory.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

// GIGS 5212: Geocentric translation (geog3D domain), EPSG Coordinate Operation
// Method 1035. GIGS transformation 61196 (GIGS geogCRS B -> GIGS geogCRS A):
// dX = 371, dY = -112, dZ = 434 metre.
//   geog3DCRS B (64019) = OSGB36  (GIGS ellipsoid B, Airy 1830)
//   geog3DCRS A (64002) = WGS 84  (GIGS ellipsoid A)
// Expected values taken from the official file GIGS_tfm_5212_..._output_EPSGconcat.

namespace
{
	// Direct B -> A (3D): reorders (lat,lon,h) -> (lon,lat,h), converts to geocentric
	// over the ellipsoid of B, applies the 3-parameter translation, converts back to
	// geographic over the ellipsoid of A and reorders (lon,lat,h) -> (lat,lon,h).
	const char* kMtD_Test5212_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6377563.396],PARAMETER["semi_minor",6356256.909237285]],PARAM_MT["geocentric_translations",PARAMETER["x_axis_translation",371],PARAMETER["y_axis_translation",-112],PARAMETER["z_axis_translation",434]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]]])WKT";

	// Inverse A -> B (3D): ellipsoids swapped and translation negated.
	const char* kMtI_Test5212_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["geocentric_translations",PARAMETER["x_axis_translation",-371],PARAMETER["y_axis_translation",112],PARAMETER["z_axis_translation",-434]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6377563.396],PARAMETER["semi_minor",6356256.909237285]],PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]]])WKT";

	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
		// Tolerancias oficiales: horizontal 0.0000003 degrees, vertical 0.01 metro.
		constexpr double sigmaAng = 3E-7;
		constexpr double sigmaLin = 1E-2;

		// FORWARD: input en B (OSGB36), esperado en A (WGS 84). Directa.
		TestTransform3D(d, 60, 120, 900, 60.00475191, 119.9952454, 619.6317, sigmaAng, sigmaLin);
		TestTransform3D(d, 60, 120, 0, 60.00475258, 119.9952447, -280.3683, sigmaAng, sigmaLin);
		TestTransform3D(d, 0, 0, 0, 0.00392509, -0.00100615, -202.5882, sigmaAng, sigmaLin);
		TestTransform3D(d, 0, 0, -3000, 0.00392695, -0.00100662, -3202.5881, sigmaAng, sigmaLin);
		TestTransform3D(d, 0, 0, -10000, 0.00393129, -0.00100773, -10202.5881, sigmaAng, sigmaLin);
		TestTransform3D(d, -60, -120, 0, -59.99934884, -119.9932376, -935.0995, sigmaAng, sigmaLin);
		TestTransform3D(d, -60, -120, -900, -59.99934874, -119.9932366, -1835.0995, sigmaAng, sigmaLin);
		TestTransform3D(d, 70, -180, 0, 70.00490733, -179.9970662, -223.6178, sigmaAng, sigmaLin);
		TestTransform3D(d, 25, -90, 0, 25.00366329, -89.99632465, -274.7286, sigmaAng, sigmaLin);
		TestTransform3D(d, -37.6532236, 143.9279419, 1836.947, -37.65282218, 143.9264925, 737.7182, sigmaAng, sigmaLin);
		TestTransform3D(d, -37.6532236, 143.9279419, 0, -37.65282206, 143.9264921, -1099.2288, sigmaAng, sigmaLin);
		TestTransform3D(d, -37.6532236, 143.9279419, -3000, -37.65282187, 143.9264914, -4099.2288, sigmaAng, sigmaLin);
		TestTransform3D(d, -37.6532236, 143.9279419, -10000, -37.65282143, 143.9264898, -11099.2288, sigmaAng, sigmaLin);
		TestTransform3D(d, -70, 180, 0, -70.00224647, -179.9970662, -1039.2896, sigmaAng, sigmaLin);

		// REVERSE: input en A (WGS 84), esperado en B (OSGB36). Inversa.
		TestTransform3D(i, 80, 150, 1214.137, 79.99575759, 150.0045612, 1350.0358, sigmaAng, sigmaLin);
		TestTransform3D(i, 80, 150, 0, 79.99575679, 150.0045621, 135.8988, sigmaAng, sigmaLin);
		TestTransform3D(i, 30, 60, 189.569, 29.99639863, 60.0039103, 450.0478, sigmaAng, sigmaLin);
		TestTransform3D(i, 30, 60, 0, 29.99639852, 60.00391042, 260.4788, sigmaAng, sigmaLin);
		TestTransform3D(i, -30, -60, 0, -30.00405481, -60.00274971, 526.4765, sigmaAng, sigmaLin);
		TestTransform3D(i, -30, -60, -526.476, -30.00405515, -60.00274993, 0, sigmaAng, sigmaLin);
		TestTransform3D(i, -30, -60, -571.476, -30.00405517, -60.00274995, -44.9995, sigmaAng, sigmaLin);
		TestTransform3D(i, -80, -150, 0, -79.99809461, -150.014563, 971.255, sigmaAng, sigmaLin);
		TestTransform3D(i, -80, -150, -971.255, -79.99809432, -150.0145652, 0, sigmaAng, sigmaLin);
		TestTransform3D(i, -80, -150, -3316.255, -79.99809363, -150.0145706, -2345, sigmaAng, sigmaLin);
		TestTransform3D(i, 50, -135, 0, 49.9955376, -135.0047634, 313.0635, sigmaAng, sigmaLin);
		TestTransform3D(i, 0, 0, 0, -0.0039251, 0.00100617, 202.6198, sigmaAng, sigmaLin);
		TestTransform3D(i, -50, 135, 0, -49.99946316, 135.0025542, 1079.7769, sigmaAng, sigmaLin);
	}
}

TEST(Test5212Part1, DISABLED_Test5212_part_1_Epsg)
{
	GTEST_SKIP() << "Not applicable: EPSG does not define a geographic-3D CRS for OSGB36 (it is a 2D datum), so the geog3D geocentric translation is not constructible via the authority factory.";
}

TEST(Test5212Part1, Test5212_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5212_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5212_part_1_MathTransform);
	ExecuteTests(d, i);
}
