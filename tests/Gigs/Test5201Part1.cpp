#include "../TestBase.h"
#include "../GigsFactory.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

// GIGS 5201: Geographic <-> Geocentric conversion (sin cambio de datum).
//   geog3DCRS A (64002) = WGS 84 geographic 3D (lat, lon, altura elipsoidal)
//   geocenCRS  A (64001) = WGS 84 geocentric (X, Y, Z)
//   GIGS ellipsoid A = WGS 84 (a = 6378137, b = 6356752.314245179).
// The official file defines FORWARD as geocentric -> geographic and REVERSE as
// geographic -> geocentric. Here "d" is geographic -> geocentric (REVERSE) and "i"
// its inverse, geocentric -> geographic (FORWARD).

namespace
{
	// d: geographic (lat,lon,h) -> geocentric (X,Y,Z). Reorders to (lon,lat,h) which
	// es lo que espera Ellipsoid_To_Geocentric.
	const char* kMtD_Test5201_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]],PARAM_MT["Ellipsoid_To_Geocentric",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]]])WKT";

	// i: geocentric (X,Y,Z) -> geographic (lat,lon,h). Geocentric_To_Ellipsoid
	// returns (lon,lat,h); the final Affine reorders to (lat,lon,h).
	const char* kMtI_Test5201_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Geocentric_To_Ellipsoid",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179]],PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]]])WKT";

	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
		// REVERSE (geographic -> geocentric): output in metres. Cartesian tolerance 0.01 m.
		constexpr double sigmaCart = 1E-2;
		// FORWARD (geocentric -> geographic): output lat/lon in degrees (tol. 0.0003 s
		// = 8.333e-8 degrees) y altura en metros (0.01 m).
		constexpr double sigmaAng = 0.0003 / 3600.0;
		constexpr double sigmaLin = 1E-2;

		// REVERSE: geographic (lat,lon,h) -> geocentric (X,Y,Z). Direct "d".
		TestTransform3D(d, 80, 150, 1214.137, -962479.592, 555687.852, 6260738.653, sigmaCart, sigmaCart);
		TestTransform3D(d, 80, 150, 0, -962297.006, 555582.435, 6259542.961, sigmaCart, sigmaCart);
		TestTransform3D(d, 30, 60, 189.569, 2764210.405, 4787752.865, 3170468.52, sigmaCart, sigmaCart);
		TestTransform3D(d, 30, 60, 0, 2764128.32, 4787610.688, 3170373.735, sigmaCart, sigmaCart);
		TestTransform3D(d, -30, -60, 0, 2764128.32, -4787610.688, -3170373.735, sigmaCart, sigmaCart);
		TestTransform3D(d, -30, -60, -526.476, 2763900.349, -4787215.831, -3170110.497, sigmaCart, sigmaCart);
		TestTransform3D(d, -30, -60, -571.476, 2763880.863, -4787182.081, -3170087.997, sigmaCart, sigmaCart);
		TestTransform3D(d, -80, -150, 0, -962297.006, -555582.435, -6259542.961, sigmaCart, sigmaCart);
		TestTransform3D(d, -80, -150, -971.255, -962150.945, -555498.107, -6258586.462, sigmaCart, sigmaCart);
		TestTransform3D(d, -80, -150, -3316.255, -961798.295, -555294.505, -6256277.087, sigmaCart, sigmaCart);
		TestTransform3D(d, 50, -135, 0, -2904698.555, -2904698.555, 4862789.038, sigmaCart, sigmaCart);
		TestTransform3D(d, 0, 0, 0, 6378137, 0, 0, sigmaCart, sigmaCart);
		TestTransform3D(d, -50, 135, 0, -2904698.555, 2904698.555, -4862789.038, sigmaCart, sigmaCart);

		// FORWARD: geocentric (X,Y,Z) -> geographic (lat,lon,h). Inverse "i".
		TestTransform3D(i, -1598248.169, 2768777.623, 5501278.468, 60.00475191, 119.9952454, 619.6317, sigmaAng, sigmaLin);
		TestTransform3D(i, -1598023.169, 2768387.912, 5500499.045, 60.00475258, 119.9952447, -280.3683, sigmaAng, sigmaLin);
		TestTransform3D(i, 6377934.396, -112, 434, 0.00392509, -0.00100615, -202.5882, sigmaAng, sigmaLin);
		TestTransform3D(i, 6374934.396, -112, 434, 0.00392695, -0.00100662, -3202.5881, sigmaAng, sigmaLin);
		TestTransform3D(i, 6367934.396, -112, 434, 0.00393129, -0.00100773, -10202.5881, sigmaAng, sigmaLin);
		TestTransform3D(i, -1598023.169, -2768611.912, -5499631.045, -59.99934884, -119.9932376, -935.0995, sigmaAng, sigmaLin);
		TestTransform3D(i, -1597798.169, -2768222.201, -5498851.622, -59.99934874, -119.9932366, -1835.0995, sigmaAng, sigmaLin);
		TestTransform3D(i, -2187336.719, -112, 5971017.093, 70.00490733, -179.9970662, -223.6178, sigmaAng, sigmaLin);
		TestTransform3D(i, 371, -5783593.614, 2679326.11, 25.00366329, -89.99632465, -274.7286, sigmaAng, sigmaLin);
		TestTransform3D(i, -4087095.478, 2977467.559, -3875457.429, -37.65282217, 143.9264925, 737.7182, sigmaAng, sigmaLin);
		TestTransform3D(i, -4085919.959, 2976611.233, -3874335.274, -37.65282206, 143.9264921, -1099.2288, sigmaAng, sigmaLin);
		TestTransform3D(i, -4084000.165, 2975212.729, -3872502.631, -37.65282187, 143.9264914, -4099.2288, sigmaAng, sigmaLin);
		TestTransform3D(i, -4079520.647, 2971949.553, -3868226.465, -37.65282143, 143.9264898, -11099.2288, sigmaAng, sigmaLin);
		TestTransform3D(i, -2187336.719, -112, -5970149.093, -70.00224647, -179.9970662, -1039.2896, sigmaAng, sigmaLin);
	}
}

TEST(Test5201Part1, Test5201_part_1_Epsg)
{
	// geog3DCRS A = EPSG 4979 (WGS 84 3D); geocenCRS A = EPSG 4978 (WGS 84 geocentric).
	// Geographic <-> geocentric conversion (same datum), without a datum operation.
	auto const geog = GetCoordinateSystemAuthorityFactory()->CreateGeographicCoordinateSystem(4979);
	auto const geocen = GetCoordinateSystemAuthorityFactory()->CreateGeocentricCoordinateSystem(4978);
	auto const [d, i] = BuildDirectInverse(geog, geocen);
	ExecuteTests(d, i);
}

TEST(Test5201Part1, Test5201_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5201_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5201_part_1_MathTransform);
	ExecuteTests(d, i);
}
