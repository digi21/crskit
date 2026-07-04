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
            TestDirectTransform(d, Sexa2DecimalDegrees(47, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(110, 0, 00.000, CardinalPoint::W), 2016617.897, 5717717.179, 2E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(43, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(110, 0, 00.000, CardinalPoint::W), 2041850.997, 4256081.225, 2E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(41, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(110, 0, 00.000, CardinalPoint::W), 2054432.46, 3527295.672, 2E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(41, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(106, 0, 00.000, CardinalPoint::W), 3157536.542, 3571750.248, 2E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(41, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(102, 0, 00.000, CardinalPoint::W), 4257426.541, 3666917.555, 2E-2);

            TestInverseTransform(i, Sexa2DecimalDegrees(49, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(110, 0, 00.000, CardinalPoint::W), 2003933.266, 6452478.797, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(45, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(110, 0, 00.000, CardinalPoint::W), 2029251.514, 4985910.592, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(41, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(110, 0, 00.000, CardinalPoint::W), 2054432.46, 3527295.672, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(41, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(108, 0, 00.000, CardinalPoint::W), 2606240.302, 3543175.461, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(41, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(104, 0, 00.000, CardinalPoint::W), 3708029.155, 3613004.897, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(47, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(110, 0, 00.000, CardinalPoint::W));
        	}

	const char* kWkt_Test5103_part_3_Wkt = R"WKT(PROJCS["GIGS projCRS G18",GEOGCS["GIGS geogCRS G",DATUM["GIGS geodetic datum G",SPHEROID["GIGS ellipsoid F",6378137,298.257222169001,AUTHORITY["GIGS","67019"]],AUTHORITY["GIGS","66007"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64010"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",40.33333329999996],PARAMETER["central_meridian",-111.4999999999999],PARAMETER["standard_parallel1",41.78333329999996],PARAMETER["standard_parallel2",40.71666669999996],PARAMETER["false_easting",500000.0001016001],PARAMETER["false_northing",999999.9998983998],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],UNIT["GIGS unit L3 (US survey foot)",0.3048006096012192,AUTHORITY["GIGS","69003"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["GIGS","62025"]])WKT";
	const char* kWkt_Test5103_part_3_WktEpsg = R"WKT(PROJCS["NAD83(HARN) / Utah North (ftUS)",GEOGCS["NAD83(HARN)",DATUM["NAD83 (High Accuracy Regional Network)",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6152"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4152"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",40.33333333333334],PARAMETER["central_meridian",-111.5],PARAMETER["standard_parallel1",41.78333333333333],PARAMETER["standard_parallel2",40.71666666666667],PARAMETER["false_easting",500000.0000101601],PARAMETER["false_northing",999999.9999898401],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314140356],UNIT["US survey foot",0.3048006096012192,AUTHORITY["EPSG","9003"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["EPSG","3568"]])WKT";
	const char* kMtD_Test5103_part_3_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Lambert_Conformal_Conic_2SP",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["standard_parallel1",41.78333329999996],PARAMETER["standard_parallel2",40.71666669999996],PARAMETER["latitude_of_origin",40.33333329999996],PARAMETER["central_meridian",-111.4999999999999],PARAMETER["false_easting",500000.0001016001],PARAMETER["false_northing",999999.9998983998]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",3.280833333333334],PARAMETER["elt_0_1",0],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",0],PARAMETER["elt_1_1",3.280833333333334],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",3.280833333333334]]])WKT";
	const char* kMtI_Test5103_part_3_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0.3048006096012192],PARAMETER["elt_0_1",0],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",0],PARAMETER["elt_1_1",0.3048006096012192],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0.3048006096012192]],INVERSE_MT[PARAM_MT["Lambert_Conformal_Conic_2SP",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["standard_parallel1",41.78333329999996],PARAMETER["standard_parallel2",40.71666669999996],PARAMETER["latitude_of_origin",40.33333329999996],PARAMETER["central_meridian",-111.4999999999999],PARAMETER["false_easting",500000.0001016001],PARAMETER["false_northing",999999.9998983998]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5103Part3, Test5103_part_3)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62025");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5103Part3, Test5103_part_3_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5103_part_3_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5103Part3, Test5103_part_3_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(3568);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5103Part3, Test5103_part_3_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5103_part_3_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5103Part3, Test5103_part_3_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5103_part_3_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5103_part_3_MathTransform);
	ExecuteTests(d, i);
}
