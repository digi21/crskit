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
            TestDirectTransform(d, Sexa2DecimalDegrees(27, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(132, 0, 00.000, CardinalPoint::E), 0, -2926820.89, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(20, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(140, 0, 00.000, CardinalPoint::E), 832799.36, -2170181.926, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(140, 0, 00.000, CardinalPoint::E), 567313.287, -6404311.163, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(130, 0, 00.000, CardinalPoint::E), -141915.257, -6387653.78, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(150, 0, 00.000, CardinalPoint::E), 1273067.747, -6476375.276, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(170, 0, 00.000, CardinalPoint::E), 2656914.716, -6784621.89, 1E-2);

            TestInverseTransform(i, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(132, 0, 00.000, CardinalPoint::E), 0, 0, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(140, 0, 00.000, CardinalPoint::E), 966973.979, -30285.601, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(40, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(140, 0, 00.000, CardinalPoint::E), 693250.209, -4395794.489, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(140, 0, 00.000, CardinalPoint::E), 486878.674, -7687130.029, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(120, 0, 00.000, CardinalPoint::E), -850274.747, -6426505.132, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(140, 0, 00.000, CardinalPoint::E), 567313.287, -6404311.163, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(160, 0, 00.000, CardinalPoint::E), 1971026.264, -6603404.818, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(27, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(132, 0, 00.000, CardinalPoint::E));
        	}

	const char* kWkt_Test5109_part_1_Wkt = R"WKT(PROJCS["GIGS projCRS F9",GEOGCS["GIGS geogCRS F",DATUM["GIGS geodetic datum F",SPHEROID["GIGS ellipsoid F",6378137,298.257222169001,AUTHORITY["GIGS","67019"]],AUTHORITY["GIGS","66006"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64009"]],PROJECTION["Albers_Conic_Equal_Area"],PARAMETER["latitude_of_center",0],PARAMETER["longitude_of_center",131.9999999999999],PARAMETER["standard_parallel1",-17.99999999999998],PARAMETER["standard_parallel2",-35.99999999999996],PARAMETER["false_easting",0],PARAMETER["false_northing",0],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["E", EAST],AXIS["N", NORTH],AUTHORITY["GIGS","62016"]])WKT";
	const char* kWkt_Test5109_part_1_WktEpsg = R"WKT(PROJCS["GDA94 / Australian Albers",GEOGCS["GDA94",DATUM["Geocentric Datum of Australia 1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4283"]],PROJECTION["Albers_Conic_Equal_Area"],PARAMETER["latitude_of_center",0],PARAMETER["longitude_of_center",131.9999999999999],PARAMETER["standard_parallel1",-17.99999999999998],PARAMETER["standard_parallel2",-35.99999999999996],PARAMETER["false_easting",0],PARAMETER["false_northing",0],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314140356],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["E", EAST],AXIS["N", NORTH],AUTHORITY["EPSG","3577"]])WKT";
	const char* kMtD_Test5109_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Albers_Conic_Equal_Area",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["standard_parallel1",-17.99999999999998],PARAMETER["standard_parallel2",-35.99999999999996],PARAMETER["latitude_of_center",0],PARAMETER["longitude_of_center",131.9999999999999],PARAMETER["false_easting",0],PARAMETER["false_northing",0]]])WKT";
	const char* kMtI_Test5109_part_1_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Albers_Conic_Equal_Area",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["standard_parallel1",-17.99999999999998],PARAMETER["standard_parallel2",-35.99999999999996],PARAMETER["latitude_of_center",0],PARAMETER["longitude_of_center",131.9999999999999],PARAMETER["false_easting",0],PARAMETER["false_northing",0]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5109Part1, Test5109_part_1)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62016");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5109Part1, Test5109_part_1_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5109_part_1_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5109Part1, Test5109_part_1_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(3577);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5109Part1, Test5109_part_1_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5109_part_1_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5109Part1, Test5109_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5109_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5109_part_1_MathTransform);
	ExecuteTests(d, i);
}
