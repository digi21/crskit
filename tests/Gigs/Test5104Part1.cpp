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
            TestDirectTransform(d, Sexa2DecimalDegrees(57, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E), 131405.466, 1002468.081, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(55, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E), 130183.562, 779577.697, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E), 128979.263, 556953.19, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(51, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 00.000, CardinalPoint::E), 57605.946, 335312.662, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(49, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 00.000, CardinalPoint::E), 53412.761, 112842.732, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 00.000, CardinalPoint::E), 61856.776, 557779.118, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(6, 0, 00.000, CardinalPoint::E), 196105.283, 557057.739, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(8, 0, 00.000, CardinalPoint::E), 330331.464, 560058.312, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(10, 0, 00.000, CardinalPoint::E), 464462.348, 566781.236, 1E-2);

            TestInverseTransform(i, Sexa2DecimalDegrees(58, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E), 132023.27, 1114054.872, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(56, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E), 130792.264, 890981.281, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(54, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E), 129579.261, 668240.578, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(52, 9, 22.178, CardinalPoint::N), Sexa2DecimalDegrees(5, 23, 15.500, CardinalPoint::E), 155000, 463000, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(50, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(4, 0, 00.000, CardinalPoint::E), 55502.306, 224086.514, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(47, 58, 30.940, CardinalPoint::N), Sexa2DecimalDegrees(3, 18, 49.421, CardinalPoint::E), 0, 0, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(3, 0, 00.000, CardinalPoint::E), -5253.063, 559535.55, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E), 128979.263, 556953.19, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(7, 0, 00.000, CardinalPoint::E), 263225.722, 558092.769, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(9, 0, 00.000, CardinalPoint::E), 397413.385, 562954.436, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(53, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(11, 0, 00.000, CardinalPoint::E), 531469.202, 571538.839, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(57, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(5, 0, 00.000, CardinalPoint::E));
        	}

	const char* kWkt_Test5104_part_1_Wkt = R"WKT(PROJCS["GIGS projCRS C4",GEOGCS["GIGS geogCRS C",DATUM["GIGS geodetic datum C",SPHEROID["GIGS ellipsoid C",6377397.155,299.152812869001,AUTHORITY["GIGS","67004"]],AUTHORITY["GIGS","66003"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64006"]],PROJECTION["Oblique_Stereographic"],PARAMETER["latitude_of_origin",52.15616059999995],PARAMETER["central_meridian",5.387638899999995],PARAMETER["scale_factor",0.9999079],PARAMETER["false_easting",155000],PARAMETER["false_northing",463000],PARAMETER["semi_major",6377397.155],PARAMETER["semi_minor",6356078.962823105],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["GIGS","62011"]])WKT";
	const char* kWkt_Test5104_part_1_WktEpsg = R"WKT(PROJCS["Amersfoort / RD New",GEOGCS["Amersfoort",DATUM["Amersfoort",SPHEROID["Bessel 1841",6377397.155,299.1528128,AUTHORITY["EPSG","7004"]],AUTHORITY["EPSG","6289"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4289"]],PROJECTION["Oblique_Stereographic"],PARAMETER["latitude_of_origin",52.15616055555555],PARAMETER["central_meridian",5.38763888888889],PARAMETER["scale_factor",0.9999079],PARAMETER["false_easting",155000],PARAMETER["false_northing",463000],PARAMETER["semi_major",6377397.155],PARAMETER["semi_minor",6356078.962818189],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["EPSG","28992"]])WKT";
	const char* kMtD_Test5104_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Oblique_Stereographic",PARAMETER["semi_major",6377397.155],PARAMETER["semi_minor",6356078.962823105],PARAMETER["latitude_of_origin",52.15616059999995],PARAMETER["central_meridian",5.387638899999995],PARAMETER["scale_factor",0.9999079],PARAMETER["false_easting",155000],PARAMETER["false_northing",463000]]])WKT";
	const char* kMtI_Test5104_part_1_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Oblique_Stereographic",PARAMETER["semi_major",6377397.155],PARAMETER["semi_minor",6356078.962823105],PARAMETER["latitude_of_origin",52.15616059999995],PARAMETER["central_meridian",5.387638899999995],PARAMETER["scale_factor",0.9999079],PARAMETER["false_easting",155000],PARAMETER["false_northing",463000]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5104Part1, Test5104_part_1)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62011");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5104Part1, Test5104_part_1_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5104_part_1_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5104Part1, Test5104_part_1_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(28992);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5104Part1, Test5104_part_1_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5104_part_1_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5104Part1, Test5104_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5104_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5104_part_1_MathTransform);
	ExecuteTests(d, i);
}
