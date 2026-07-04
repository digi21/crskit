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
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), 221288.770, 6661953.041, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(20, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), -23538.687, 2219308.238, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), -57087.120, 0.000, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(40, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), 73106.698, -4439746.917, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), 403186.945, -8885748.708, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::W), 54506.435, 6678411.623, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 0, 00.000, CardinalPoint::W), 165640.332, 6666593.572, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(1, 0, 00.000, CardinalPoint::W), 276979.926, 6658157.202, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(1, 0, 00.000, CardinalPoint::E), 388455.958, 6653097.435, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 0, 00.000, CardinalPoint::E), 500000.000, 6651411.190, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E), 611544.042, 6653097.435, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(7, 0, 00.000, CardinalPoint::E), 723020.074, 6658157.202, 1E-2);

            TestInverseTransform(i, Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), 403186.945, 8885748.708, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(40, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), 73106.698, 4439746.917, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(20, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), -23538.687, -2219308.238, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), 221288.770, -6661953.041, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 00.000, CardinalPoint::W), 110043.299, 6672079.494, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::W), 221288.770, 6661953.041, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::E), 332705.179, 6655205.484, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(2, 0, 00.000, CardinalPoint::E), 444223.733, 6651832.735, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 00.000, CardinalPoint::E), 555776.267, 6651832.735, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(6, 0, 00.000, CardinalPoint::E), 667294.821, 6655205.484, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(8, 0, 00.000, CardinalPoint::E), 778711.230, 6661953.041, 1E-6);
	}

	const char* kWkt_Test5101_part_2_Wkt = R"WKT(PROJCS["GIGS projCRS A1",GEOGCS["GIGS geogCRS A",DATUM["GIGS geodetic datum A",SPHEROID["GIGS ellipsoid A",6378137,298.2572236,AUTHORITY["GIGS","67030"]],AUTHORITY["GIGS","66001"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64003"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",2.999999999999997],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314247833],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["N", EAST],AXIS["N", NORTH],AUTHORITY["GIGS","62001"]])WKT";
	const char* kWkt_Test5101_part_2_WktEpsg = R"WKT(PROJCS["WGS 84 / UTM zone 31N",GEOGCS["WGS 84",DATUM["World Geodetic System 1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4326"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",2.999999999999997],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314245179],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["E", EAST],AXIS["N", NORTH],AUTHORITY["EPSG","32631"]])WKT";
	const char* kMtD_Test5101_part_2_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Transverse_Mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314247833],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",2.999999999999997],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0]]])WKT";
	const char* kMtI_Test5101_part_2_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Transverse_Mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314247833],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",2.999999999999997],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5101Part2, Test5101_part_2)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62001");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part2, Test5101_part_2_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5101_part_2_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part2, Test5101_part_2_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(32631);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part2, Test5101_part_2_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5101_part_2_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part2, Test5101_part_2_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5101_part_2_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5101_part_2_MathTransform);
	ExecuteTests(d, i);
}
