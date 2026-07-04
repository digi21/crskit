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
            TestDirectTransform(d, Sexa2DecimalDegrees(12, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 807919.144, 1329535.334, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(9, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 809334.177, 996918.212, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(6, 52, 42.450, CardinalPoint::N), Sexa2DecimalDegrees(116, 50, 47.588, CardinalPoint::E), 793704.631, 762081.047, 2E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 811930.345, 554475.627, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(3, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 813245.133, 333300.13, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(1, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 814401.375, 111916.452, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(123, 0, 0, CardinalPoint::E), 1475669.281, 673118.573, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(121, 0, 0, CardinalPoint::E), 1254086.173, 669446.249, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(119, 0, 0, CardinalPoint::E), 1032643.312, 666797.354, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 811253.303, 665041.265, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(115, 0, 0, CardinalPoint::E), 589825.706, 664048.715, 1E-2);

            TestInverseTransform(i, Sexa2DecimalDegrees(10, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 808784.981, 1107678.473, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(10, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 808784.981, 1107678.473, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(8, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 809939.302, 886240.183, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 811253.303, 665041.265, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 812599.582, 443902.706, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(115, 0, 0, CardinalPoint::E), 590521.147, 442890.861, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(2, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E), 813851.067, 222645.511, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(0, 0, 0.624, CardinalPoint::S), Sexa2DecimalDegrees(109, 41, 8.955, CardinalPoint::E), 0, 0, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(122, 0, 0, CardinalPoint::E), 1364854.862, 671146.254, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(120, 0, 0, CardinalPoint::E), 1143352.598, 668002.074, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(118, 0, 0, CardinalPoint::E), 921947.286, 665815.815, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(116, 0, 0, CardinalPoint::E), 700549.965, 664457.586, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(114, 0, 0, CardinalPoint::E), 479068.802, 663798.63, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(12, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(117, 0, 0, CardinalPoint::E));
        	}

	const char* kWkt_Test5105_part_1_Wkt = R"WKT(PROJCS["GIGS projCRS G13",GEOGCS["GIGS geogCRS G",DATUM["GIGS geodetic datum G",SPHEROID["GIGS ellipsoid F",6378137,298.257222169001,AUTHORITY["GIGS","67019"]],AUTHORITY["GIGS","66007"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64010"]],PROJECTION["Oblique_Mercator"],PARAMETER["latitude_of_center",3.999999999999996],PARAMETER["longitude_of_center",114.9999999999999],PARAMETER["azimuth",53.31580993999995],PARAMETER["rectified_grid_angle",53.13010235999995],PARAMETER["scale_factor",0.99984],PARAMETER["false_easting",590521.147],PARAMETER["false_northing",442890.861],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["E", EAST],AXIS["N", NORTH],AUTHORITY["GIGS","62020"]])WKT";
	const char* kMtD_Test5105_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["oblique_mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["latitude_of_center",3.999999999999996],PARAMETER["longitude_of_center",114.9999999999999],PARAMETER["azimuth",53.31580993999995],PARAMETER["rectified_grid_angle",53.13010235999995],PARAMETER["scale_factor",0.99984],PARAMETER["false_easting",590521.147],PARAMETER["false_northing",442890.861]]])WKT";
	const char* kMtI_Test5105_part_1_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["oblique_mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["latitude_of_center",3.999999999999996],PARAMETER["longitude_of_center",114.9999999999999],PARAMETER["azimuth",53.31580993999995],PARAMETER["rectified_grid_angle",53.13010235999995],PARAMETER["scale_factor",0.99984],PARAMETER["false_easting",590521.147],PARAMETER["false_northing",442890.861]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5105Part1, Test5105_part_1)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62020");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5105Part1, Test5105_part_1_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5105_part_1_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5105Part1, Test5105_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5105_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5105_part_1_MathTransform);
	ExecuteTests(d, i);
}
