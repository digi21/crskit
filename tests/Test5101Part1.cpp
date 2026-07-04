// Ported from TestDigi21OpenGIS/Test5101Part1.cs (GIGS series 5101, part 1).
// Transverse Mercator projection. Data according to the JHS formulas (#define _FORMULAS_JHS).

#include "TestBase.h"
#include "GigsFactory.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	// Block of checks shared by the three variants (equivalent to the
	// override ExecuteTests(IMathTransform d, IMathTransform i) del C#).
	void ExecuteTests(MathTransform const& d, MathTransform const& i)
	{
		TestDirectTransform(d, Sexa2DecimalDegrees(80, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 496813.178, 3358297.326, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(49, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 765648.501, -87944.740, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(20, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 923539.353, -3308151.625, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(20, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 923539.353, -7746773.748, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 678711.584, -12189424.202, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(80, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 496813.178, -14413222.698, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, 1123956.966, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(40, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -1099699.834, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -5527462.686, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(40, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -9955225.538, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(80, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -14409059.801, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(80, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 341867.711, 3355633.571, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(49, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 180586.020, -95662.911, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(20, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 86073.280, -3313165.843, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(20, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 86073.280, -7741759.529, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 232704.966, -12182676.637, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 66021.018, -5527462.686, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::W), 288719.208, -5527462.686, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(1, 00, 00.000, CardinalPoint::W), 511280.792, -5527462.686, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(1, 00, 00.000, CardinalPoint::E), 733978.982, -5527462.686, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 957087.829, -5527462.686, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::E), 1180883.933, -5527462.686, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(7, 00, 00.000, CardinalPoint::E), 1405648.179, -5527462.686, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 232704.966, 1127751.264, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::W), 344223.662, 1124378.512, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(1, 00, 00.000, CardinalPoint::W), 455776.338, 1124378.512, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(1, 00, 00.000, CardinalPoint::E), 567295.034, 1127751.264, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 678711.584, 1134498.830, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::E), 789957.197, 1144625.296, 1E-2);
		TestDirectTransform(d, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(7, 00, 00.000, CardinalPoint::E), 900962.042, 1158136.713, 1E-2);

		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 678711.584, 1134498.830, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(40, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 826893.845, -1087710.121, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 957087.829, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(40, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 826893.845, -9967215.251, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(80, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E), 496813.178, -14413222.698, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(80, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, 3354134.429, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(49, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -100000.000, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(20, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -3315978.565, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(20, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -7738946.807, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -12178882.338, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 232704.966, 1127751.264, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(40, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 143900.026, -1095387.991, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 66021.018, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(40, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 143900.026, -9959537.381, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(80, 00, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(5, 00, 00.000, CardinalPoint::W), 341867.711, -14410558.943, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(49, 45, 58.078, CardinalPoint::N), Sexa2DecimalDegrees(7, 33, 21.253, CardinalPoint::W), 0.000, 0.000, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 00, 00.000, CardinalPoint::W), 177404.277, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::E), 622595.723, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::E), 845464.865, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 00, 00.000, CardinalPoint::E), 1068882.539, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(6, 00, 00.000, CardinalPoint::E), 1293127.266, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(8, 00, 00.000, CardinalPoint::E), 1518482.747, -5527462.686, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 00, 00.000, CardinalPoint::W), 288455.816, 1125643.213, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::W), 400000.000, 1123956.966, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(0, 00, 00.000, CardinalPoint::E), 511544.184, 1125643.213, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 00, 00.000, CardinalPoint::E), 623020.357, 1130702.987, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 00, 00.000, CardinalPoint::E), 734360.093, 1139139.367, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(6, 00, 00.000, CardinalPoint::E), 845494.132, 1150957.434, 1E-6);
		TestInverseTransform(i, Sexa2DecimalDegrees(60, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(8, 00, 00.000, CardinalPoint::E), 956351.967, 1166164.180, 1E-6);

		ExecuteIterations(i, d, Sexa2DecimalDegrees(80, 00, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 00, 00.000, CardinalPoint::E));
	}

	const char* kWkt =
		R"WKT(PROJCS["GIGS projCRS A2",GEOGCS["GIGS geogCRS A",DATUM["GIGS geodetic datum A",SPHEROID["GIGS ellipsoid A",6378137,298.2572236,AUTHORITY["GIGS","67030"]],AUTHORITY["GIGS","66001"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64003"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",48.99999999999995],PARAMETER["central_meridian",-1.999999999999998],PARAMETER["scale_factor",0.999601272],PARAMETER["false_easting",400000],PARAMETER["false_northing",-100000],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314247833],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["E", EAST],AXIS["N", NORTH],AUTHORITY["GIGS","62007"]])WKT";

	const char* kMathTransformDirecta =
		R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Transverse_Mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314247833],PARAMETER["latitude_of_origin",48.99999999999995],PARAMETER["central_meridian",-1.999999999999998],PARAMETER["scale_factor",0.999601272],PARAMETER["false_easting",400000],PARAMETER["false_northing",-100000]]])WKT";

	const char* kMathTransformInversa =
		R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Transverse_Mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314247833],PARAMETER["latitude_of_origin",48.99999999999995],PARAMETER["central_meridian",-1.999999999999998],PARAMETER["scale_factor",0.999601272],PARAMETER["false_easting",400000],PARAMETER["false_northing",-100000]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5101Part1, Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);

	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part1, MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMathTransformDirecta);
	auto const i = mtf->CreateFromWkt(kMathTransformInversa);
	ExecuteTests(d, i);
}

TEST(Test5101Part1, GigsFactory)
{
	Gigs::GigsFactory gigsFactory;
	auto const pcs = gigsFactory.CreateProjectedCoordinateSystem("62007");
	auto const gcs = pcs->GetGeographicCoordinateSystem();

	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}
