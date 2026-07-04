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
            TestDirectTransform(d, Sexa2DecimalDegrees(58, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 760722.92, 3457368.68, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(56, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 768396.68, 3230944.81, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(54, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 776020.19, 3006003.84, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(51, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 717027.29, 2668695.78, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(49, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 721740.43, 2445941.16, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(3, 0, 0, CardinalPoint::E), 644764.91, 2891124.20, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 779816.75, 2893981.68, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(7, 0, 0, CardinalPoint::E), 914752.17, 2900274.37, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(9, 0, 0, CardinalPoint::E), 1049483.80, 2909998.20, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(11, 0, 0, CardinalPoint::E), 1183924.41, 2923146.86, 1E-2);

            TestInverseTransform(i, Sexa2DecimalDegrees(57, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 764566.84, 3343948.93, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(55, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 772213.97, 3118310.95, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E), 779816.75, 2893981.68, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(50, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 719385.25, 2557252.84, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(46, 48, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 726915.75, 2201342.52, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 0, CardinalPoint::E), 712299.92, 2892123.37, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::E), 847304.47, 2896698.83, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(8, 0, 0, CardinalPoint::E), 982148.91, 2904707.73, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(10, 0, 0, CardinalPoint::E), 1116745.93, 2916144.90, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(58, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 0, CardinalPoint::E));
        	}

	const char* kWkt_Test5102_part_1_Wkt = R"WKT(PROJCS["GIGS projCRS M25",GEOGCS["GIGS geogCRS M",DATUM["GIGS geodetic datum M",SPHEROID["GIGS ellipsoid E",6378388,297,AUTHORITY["GIGS","67022"]],AUTHORITY["GIGS","66016"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64020"]],PROJECTION["Lambert_Conformal_Conic_1SP"],PARAMETER["latitude_of_origin",46.79999999999995],PARAMETER["central_meridian",2.337229199999998],PARAMETER["scale_factor",0.99987742],PARAMETER["false_easting",600000],PARAMETER["false_northing",2200000],PARAMETER["semi_major",6378388],PARAMETER["semi_minor",6356911.946127947],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["GIGS","62035"]])WKT";
	const char* kWkt_Test5102_part_1_WktEpsg = R"WKT(PROJCS["ED50 / France EuroLambert",GEOGCS["ED50",DATUM["European Datum 1950",SPHEROID["International 1924",6378388,297,AUTHORITY["EPSG","7022"]],AUTHORITY["EPSG","6230"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4230"]],PROJECTION["Lambert_Conformal_Conic_1SP"],PARAMETER["latitude_of_origin",46.79999999999999],PARAMETER["central_meridian",2.337229166666667],PARAMETER["scale_factor",0.99987742],PARAMETER["false_easting",600000],PARAMETER["false_northing",2200000],PARAMETER["semi_major",6378388],PARAMETER["semi_minor",6356911.946127947],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["EPSG","2192"]])WKT";
	const char* kMtD_Test5102_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Lambert_Conformal_Conic_1SP",PARAMETER["semi_major",6378388],PARAMETER["semi_minor",6356911.946127947],PARAMETER["latitude_of_origin",46.79999999999995],PARAMETER["central_meridian",2.337229199999998],PARAMETER["scale_factor",0.99987742],PARAMETER["false_easting",600000],PARAMETER["false_northing",2200000]]])WKT";
	const char* kMtI_Test5102_part_1_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Lambert_Conformal_Conic_1SP",PARAMETER["semi_major",6378388],PARAMETER["semi_minor",6356911.946127947],PARAMETER["latitude_of_origin",46.79999999999995],PARAMETER["central_meridian",2.337229199999998],PARAMETER["scale_factor",0.99987742],PARAMETER["false_easting",600000],PARAMETER["false_northing",2200000]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5102Part1, Test5102_part_1)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62035");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5102Part1, Test5102_part_1_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5102_part_1_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5102Part1, Test5102_part_1_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(2192);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5102Part1, Test5102_part_1_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5102_part_1_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5102Part1, Test5102_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5102_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5102_part_1_MathTransform);
	ExecuteTests(d, i);
}
