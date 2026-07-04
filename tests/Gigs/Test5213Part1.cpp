#include "../TestBase.h"
#include "../GigsFactory.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

// GIGS 5213: Geocentric translation (geog2D domain), EPSG Coordinate Operation
// Method 9603. GIGS transformation 61196 (GIGS geogCRS B -> GIGS geogCRS A):
// dX = 371, dY = -112, dZ = 434 metre.
//   geogCRS B (64005) = OSGB36  (GIGS ellipsoid B, Airy 1830)
//   geogCRS A (64003) = WGS 84  (GIGS ellipsoid A)
// Expected values taken from the official file GIGS_tfm_5213_..._output_EPSGconcat
// (the geocentric-concatenation variant, which is the one the library implements;
// the AbrMol -Abridged Molodensky- variant differs and is not applicable here).

namespace
{
	// Direct B -> A: reorders (lat,lon) -> (lon,lat,0), converts to geocentric over the
	// ellipsoid of B, applies the 3-parameter translation, converts back to geographic over
	// the ellipsoid of A and reorders (lon,lat) -> (lat,lon).
	const char* kMtD_Test5213_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6377563.396],PARAMETER["semi_minor",6356256.909237285]],PARAM_MT["geocentric_translations",PARAMETER["x_axis_translation",371],PARAMETER["y_axis_translation",-112],PARAMETER["z_axis_translation",434]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_2_3",1]]])WKT";

	// Inverse A -> B: same structure with the ellipsoids swapped and the
	// translation negated (the inverse of a geocentric translation). Built by
	// by hand because the reordering Affines (4x3 / 3x4) are not square and cannot
	// admiten GetInverse().
	const char* kMtI_Test5213_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["geocentric_translations",PARAMETER["x_axis_translation",-371],PARAMETER["y_axis_translation",112],PARAMETER["z_axis_translation",-434]],PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6377563.396],PARAMETER["semi_minor",6356256.909237285]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0],PARAMETER["elt_2_3",1]]])WKT";

	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
		// Official geographic tolerance: 0.0000003 degrees.
		constexpr double sigma = 3E-7;

		// FORWARD: input in B (OSGB36), expected in A (WGS 84). Uses the direct one.
		TestDirectTransform(d, 60, 120, 60.00475258, 119.9952447, sigma);
		TestDirectTransform(d, 0, 0, 0.00392509, -0.00100615, sigma);
		TestDirectTransform(d, -60, -120, -59.99934884, -119.9932376, sigma);
		TestDirectTransform(d, 70, -180, 70.00490733, -179.9970662, sigma);
		TestDirectTransform(d, 25, -90, 25.00366329, -89.99632465, sigma);
		TestDirectTransform(d, -37.6532236, 143.9279419, -37.65282206, 143.9264921, sigma);
		TestDirectTransform(d, -70, 180, -70.00224647, -179.9970662, sigma);

		// REVERSE: input in A (WGS 84), expected in B (OSGB36). Uses the inverse.
		TestInverseTransform(i, 79.99575679, 150.0045621, 80, 150, sigma);
		TestInverseTransform(i, 29.99639852, 60.00391042, 30, 60, sigma);
		TestInverseTransform(i, -30.00405481, -60.00274971, -30, -60, sigma);
		TestInverseTransform(i, -79.99809461, -150.014563, -80, -150, sigma);
		TestInverseTransform(i, 49.9955376, -135.0047634, 50, -135, sigma);
		TestInverseTransform(i, -0.0039251, 0.00100617, 0, 0, sigma);
		TestInverseTransform(i, -49.99946316, 135.0025542, -50, 135, sigma);

		ExecuteIterations(d, i, 60, 120, 2E-4);
	}
}

TEST(Test5213Part1, Test5213_part_1_Epsg)
{
	// geogCRS B = EPSG 4277 (OSGB36); geogCRS A = EPSG 4326 (WGS 84).
	// Operation 1196 "OSGB36 to WGS 84 (2)": geocentric translation dX=371, dY=-112,
	// dZ=434 (method 9603), which matches the GIGS parameters of this test.
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4277);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326);
	auto const [d, i] = BuildDirectInverseWithOperation(source, target, 1196);
	ExecuteTests(d, i);
}

TEST(Test5213Part1, Test5213_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5213_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5213_part_1_MathTransform);
	ExecuteTests(d, i);
}
