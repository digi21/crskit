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
            TestDirectTransform(d, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(51, 0, 00.000, CardinalPoint::E), 0.00, 0.00, 3E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(20, 30, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(54, 0, 00.000, CardinalPoint::E), 1724781.50, 248556.44, 3E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(41, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(67, 0, 00.000, CardinalPoint::E), -3709687.25, 1325634.35, 3E-2);

            TestInverseTransform(i, Sexa2DecimalDegrees(42, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(51, 0, 00.000, CardinalPoint::E), 3819897.85, 0.00, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(57, 0, 00.000, CardinalPoint::E), 0.00, 497112.88, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(51, 0, 00.000, CardinalPoint::E));
        	}

	const char* kWkt_Test5112_part_1_Wkt = R"WKT(PROJCS["GIGS projCRS Y24",GEOGCS["GIGS geogCRS Y",DATUM["GIGS geodetic datum Y",SPHEROID["GIGS ellipsoid Y",6378245,298.3,AUTHORITY["GIGS","67024"]],AUTHORITY["GIGS","66014"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64017"]],PROJECTION["Mercator_2SP"],PARAMETER["standard_parallel_1",41.99999999999996],PARAMETER["central_meridian",50.99999999999995],PARAMETER["false_easting",0],PARAMETER["false_northing",0],PARAMETER["semi_major",6378245],PARAMETER["semi_minor",6356863.018773047],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["Y", NORTH],AXIS["X", EAST],AUTHORITY["GIGS","62034"]])WKT";
	const char* kWkt_Test5112_part_1_WktEpsg = R"WKT(PROJCS["Pulkovo 1942 / Caspian Sea Mercator",GEOGCS["Pulkovo 1942",DATUM["Pulkovo 1942",SPHEROID["Krassowsky 1940",6378245,298.3,AUTHORITY["EPSG","7024"]],AUTHORITY["EPSG","6284"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4284"]],PROJECTION["Mercator_2SP"],PARAMETER["standard_parallel_1",41.99999999999996],PARAMETER["central_meridian",50.99999999999995],PARAMETER["false_easting",0],PARAMETER["false_northing",0],PARAMETER["semi_major",6378245],PARAMETER["semi_minor",6356863.018773047],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["none", NORTH],AXIS["none", EAST],AUTHORITY["EPSG","3388"]])WKT";
	const char* kMtD_Test5112_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Mercator_2SP",PARAMETER["semi_major",6378245],PARAMETER["semi_minor",6356863.018773047],PARAMETER["standard_parallel_1",41.99999999999996],PARAMETER["central_meridian",50.99999999999995],PARAMETER["false_easting",0],PARAMETER["false_northing",0]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
	const char* kMtI_Test5112_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],INVERSE_MT[PARAM_MT["Mercator_2SP",PARAMETER["semi_major",6378245],PARAMETER["semi_minor",6356863.018773047],PARAMETER["standard_parallel_1",41.99999999999996],PARAMETER["central_meridian",50.99999999999995],PARAMETER["false_easting",0],PARAMETER["false_northing",0]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5112Part1, Test5112_part_1)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62034");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5112Part1, Test5112_part_1_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5112_part_1_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5112Part1, Test5112_part_1_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(3388);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5112Part1, Test5112_part_1_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5112_part_1_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5112Part1, Test5112_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5112_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5112_part_1_MathTransform);
	ExecuteTests(d, i);
}
