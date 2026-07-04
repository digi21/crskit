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
            TestDirectTransform(d, Sexa2DecimalDegrees(48, 30, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(16, 0, 00.000, CardinalPoint::E), 424714.235, 355124.6, 2E-1);
            TestDirectTransform(d, Sexa2DecimalDegrees(47, 38, 10.085, CardinalPoint::N), Sexa2DecimalDegrees(17, 34, 57.542, CardinalPoint::E), 539847.765, 255701.086, 2E-1);
            TestDirectTransform(d, Sexa2DecimalDegrees(46, 52, 32.406, CardinalPoint::N), Sexa2DecimalDegrees(19, 13, 24.346, CardinalPoint::E), 663329.053, 170142.318, 2E-1);
            TestDirectTransform(d, Sexa2DecimalDegrees(45, 42, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(21, 24, 00.000, CardinalPoint::E), 833148.855, 42191.482, 2E-1);
            TestDirectTransform(d, Sexa2DecimalDegrees(48, 29, 23.909, CardinalPoint::N), Sexa2DecimalDegrees(21, 17, 39.115, CardinalPoint::E), 815999.993, 351999.998, 2E-1);
            TestDirectTransform(d, Sexa2DecimalDegrees(46, 4, 07.487, CardinalPoint::N), Sexa2DecimalDegrees(17, 37, 08.953, CardinalPoint::E), 539403.958, 81440.103, 2E-1);

            TestInverseTransform(i, Sexa2DecimalDegrees(48, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(17, 12, 00.000, CardinalPoint::E), 512056.188, 296756.716, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(47, 8, 39.817, CardinalPoint::N), Sexa2DecimalDegrees(19, 2, 54.858, CardinalPoint::E), 650000, 200000, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(46, 22, 13.084, CardinalPoint::N), Sexa2DecimalDegrees(20, 8, 08.666, CardinalPoint::E), 733651.455, 114532.099, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(49, 18, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(22, 18, 00.000, CardinalPoint::E), 886565.935, 444656.613, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(46, 52, 32.406, CardinalPoint::N), Sexa2DecimalDegrees(19, 13, 24.346, CardinalPoint::E), 663329.053, 170142.318, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(45, 30, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(16, 21, 36.000, CardinalPoint::E), 439836.709, 20816.456, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(48, 30, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(16, 0, 00.000, CardinalPoint::E));
        	}

	const char* kWkt_Test5105_part_2_Wkt = R"WKT(PROJCS["GIGS projCRS K26",GEOGCS["GIGS geogCRS K",DATUM["GIGS geodetic datum K",SPHEROID["GIGS ellipsoid K",6378160,298.247167469001,AUTHORITY["GIGS","67036"]],AUTHORITY["GIGS","66012"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64015"]],PROJECTION["Oblique_Mercator"],PARAMETER["latitude_of_center",47.14439372222217],PARAMETER["longitude_of_center",19.04857177777776],PARAMETER["azimuth",89.99999999999992],PARAMETER["rectified_grid_angle",89.99999999999992],PARAMETER["scale_factor",0.99993],PARAMETER["false_easting",650000],PARAMETER["false_northing",200000],PARAMETER["semi_major",6378160],PARAMETER["semi_minor",6356774.516093726],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["Y", EAST],AXIS["X", NORTH],AUTHORITY["GIGS","62036"]])WKT";
	const char* kWkt_Test5105_part_2_WktEpsg = R"WKT(PROJCS["HD72 / EOV",GEOGCS["HD72",DATUM["Hungarian Datum 1972",SPHEROID["GRS 1967",6378160,298.247167427,AUTHORITY["EPSG","7036"]],AUTHORITY["EPSG","6237"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4237"]],PROJECTION["oblique_mercator"],PARAMETER["latitude_of_center",47.14439372222222],PARAMETER["longitude_of_center",19.04857177777778],PARAMETER["azimuth",90],PARAMETER["rectified_grid_angle",90],PARAMETER["scale_factor",0.99993],PARAMETER["false_easting",650000],PARAMETER["false_northing",200000],PARAMETER["semi_major",6378160],PARAMETER["semi_minor",6356774.516090714],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Y", EAST],AXIS["X", NORTH],AUTHORITY["EPSG","23700"]])WKT";
	const char* kMtD_Test5105_part_2_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["oblique_mercator",PARAMETER["semi_major",6378160],PARAMETER["semi_minor",6356774.516093726],PARAMETER["latitude_of_center",47.14439372222217],PARAMETER["longitude_of_center",19.04857177777776],PARAMETER["azimuth",89.99999999999992],PARAMETER["rectified_grid_angle",89.99999999999992],PARAMETER["scale_factor",0.99993],PARAMETER["false_easting",650000],PARAMETER["false_northing",200000]]])WKT";
	const char* kMtI_Test5105_part_2_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["oblique_mercator",PARAMETER["semi_major",6378160],PARAMETER["semi_minor",6356774.516093726],PARAMETER["latitude_of_center",47.14439372222217],PARAMETER["longitude_of_center",19.04857177777776],PARAMETER["azimuth",89.99999999999992],PARAMETER["rectified_grid_angle",89.99999999999992],PARAMETER["scale_factor",0.99993],PARAMETER["false_easting",650000],PARAMETER["false_northing",200000]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5105Part2, Test5105_part_2)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62036");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5105Part2, Test5105_part_2_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5105_part_2_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5105Part2, Test5105_part_2_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(23700);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5105Part2, Test5105_part_2_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5105_part_2_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5105Part2, Test5105_part_2_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5105_part_2_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5105_part_2_MathTransform);
	ExecuteTests(d, i);
}
