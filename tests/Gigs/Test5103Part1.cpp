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
            TestDirectTransform(d, Sexa2DecimalDegrees(58, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 187742.7, 969521.653, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(56, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 189652.853, 745291.184, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(54, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 191555.55, 521935.9, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(52, 9, 22.178, CardinalPoint::N), Sexa2DecimalDegrees(5, 23, 15.500, CardinalPoint::E), 219843.841, 316827.604, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(50, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 123652.406, 76521.628, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(47, 58, 30.940, CardinalPoint::N), Sexa2DecimalDegrees(3, 18, 49.421, CardinalPoint::E), 71254.553, -148236.592, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(3, 0, 0, CardinalPoint::E), 58108.966, 411155.591, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 192504.921, 410490.433, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(7, 0, 0, CardinalPoint::E), 326870.04, 413445.087, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(9, 0, 0, CardinalPoint::E), 461106.844, 420017.408, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(11, 0, 0, CardinalPoint::E), 595117.95, 430202.63, 1E-3);

            TestInverseTransform(i, Sexa2DecimalDegrees(57, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 188698.877, 857277.135, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(55, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 190604.967, 633523.672, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 192504.921, 410490.433, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(51, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 124202.936, 187756.876, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(49, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 123101.889, -34711.068, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 125304.704, 410370.504, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::E), 259697.429, 411515.356, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(8, 0, 0, CardinalPoint::E), 394010.571, 416279.276, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(10, 0, 0, CardinalPoint::E), 528146.69, 424658.807, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(58, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E));
        	}

	const char* kWkt_Test5103_part_1_Wkt = R"WKT(PROJCS["GIGS projCRS E6",GEOGCS["GIGS geogCRS E",DATUM["GIGS geodetic datum E",SPHEROID["GIGS ellipsoid E",6378388,297,AUTHORITY["GIGS","67022"]],AUTHORITY["GIGS","66005"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64008"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",89.99999999999992],PARAMETER["central_meridian",4.367486666999995],PARAMETER["standard_parallel1",51.16666722999996],PARAMETER["standard_parallel2",49.83333389999995],PARAMETER["false_easting",150000.013],PARAMETER["false_northing",5400088.438],PARAMETER["semi_major",6378388],PARAMETER["semi_minor",6356911.946127947],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["GIGS","62013"]])WKT";
	const char* kWkt_Test5103_part_1_WktEpsg = R"WKT(PROJCS["Belge 1972 / Belgian Lambert 72",GEOGCS["Belge 1972",DATUM["Reseau National Belge 1972",SPHEROID["International 1924",6378388,297,AUTHORITY["EPSG","7022"]],AUTHORITY["EPSG","6313"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4313"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",90],PARAMETER["central_meridian",4.367486666666666],PARAMETER["standard_parallel1",51.16666723333333],PARAMETER["standard_parallel2",49.83333389999999],PARAMETER["false_easting",150000.013],PARAMETER["false_northing",5400088.438],PARAMETER["semi_major",6378388],PARAMETER["semi_minor",6356911.946127947],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["EPSG","31370"]])WKT";
	const char* kMtD_Test5103_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Lambert_Conformal_Conic_2SP",PARAMETER["semi_major",6378388],PARAMETER["semi_minor",6356911.946127947],PARAMETER["standard_parallel1",51.16666722999996],PARAMETER["standard_parallel2",49.83333389999995],PARAMETER["latitude_of_origin",89.99999999999992],PARAMETER["central_meridian",4.367486666999995],PARAMETER["false_easting",150000.013],PARAMETER["false_northing",5400088.438]]])WKT";
	const char* kMtI_Test5103_part_1_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Lambert_Conformal_Conic_2SP",PARAMETER["semi_major",6378388],PARAMETER["semi_minor",6356911.946127947],PARAMETER["standard_parallel1",51.16666722999996],PARAMETER["standard_parallel2",49.83333389999995],PARAMETER["latitude_of_origin",89.99999999999992],PARAMETER["central_meridian",4.367486666999995],PARAMETER["false_easting",150000.013],PARAMETER["false_northing",5400088.438]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5103Part1, Test5103_part_1)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62013");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5103Part1, Test5103_part_1_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5103_part_1_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5103Part1, Test5103_part_1_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(31370);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5103Part1, Test5103_part_1_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5103_part_1_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5103Part1, Test5103_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5103_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5103_part_1_MathTransform);
	ExecuteTests(d, i);
}
