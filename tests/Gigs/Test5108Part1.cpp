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
            TestDirectTransform(d, 9, 106, 268006.024, 770398.186, 1E-2);
            TestDirectTransform(d, 7, 106, 269388.786, 548990.588, 1E-2);
            TestDirectTransform(d, 5, 106, 270427.255, 327597.962, 1E-2);
            TestDirectTransform(d, 3, 106, 271120.234, 106216.081, 1E-2);
            TestDirectTransform(d, 1, 106, 271466.923, -115159.332, 1E-2);
            TestDirectTransform(d, 5, 109, 603116.703, 329668.5989, 3E-2);
            TestDirectTransform(d, 5, 107, 381324.7402, 328117.4715, 1E-2);
            TestDirectTransform(d, 5, 105, 159529.111, 327248.012, 1E-2);

            TestInverseTransform(i, 10, 106, 267186.017, 881108.902, 1E-4);
            TestInverseTransform(i, 8, 106, 268740.351, 659692.254, 1E-4);
            TestInverseTransform(i, 6, 106, 269951.141, 438292.666, 1E-4);
            TestInverseTransform(i, 4, 106, 270816.99, 216905.945, 1E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(2, 2, 32.884, CardinalPoint::N), Sexa2DecimalDegrees(103, 33, 39.837, CardinalPoint::E), 0, 0, 1E-4);
            TestInverseTransform(i, Sexa2DecimalDegrees(1, 49, 39.954, CardinalPoint::N), Sexa2DecimalDegrees(103, 38, 24.935, CardinalPoint::E), 8813.252, -23740.095, 1E-4);
            TestInverseTransform(i, 5, 108, 492221.308256457, 328807.336254484, 1E-4);
            TestInverseTransform(i, 5, 106, 270427.255, 327597.962, 1E-4);
            TestInverseTransform(i, 5, 104, 48630.563, 327067.097, 1E-4);

            ExecuteIterations(d, i, 9, 106, 1E-1);
        	}

	const char* kWkt_Test5108_part_1_Wkt = R"WKT(PROJCS["GIGS projCRS G15",GEOGCS["GIGS geogCRS G",DATUM["GIGS geodetic datum G",SPHEROID["GIGS ellipsoid F",6378137,298.257222169001,AUTHORITY["GIGS","67019"]],AUTHORITY["GIGS","66007"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64010"]],PROJECTION["Cassini_Soldner"],PARAMETER["latitude_of_origin",2.121679739999998],PARAMETER["central_meridian",103.4279362360999],PARAMETER["false_easting",-14810.56199999999],PARAMETER["false_northing",8758.319999999991],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["GIGS","62022"]])WKT";
	const char* kWkt_Test5108_part_1_WktEpsg = R"WKT(PROJCS["GDM2000 / Johor Grid",GEOGCS["GDM2000",DATUM["Geodetic Datum of Malaysia 2000",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6742"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4742"]],PROJECTION["Cassini_Soldner"],PARAMETER["latitude_of_origin",2.121679744444445],PARAMETER["central_meridian",103.4279362361111],PARAMETER["false_easting",-14810.562],PARAMETER["false_northing",8758.32],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314140356],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["E", EAST],AXIS["N", NORTH],AUTHORITY["EPSG","3377"]])WKT";
	const char* kMtD_Test5108_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Cassini_Soldner",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["latitude_of_origin",2.121679739999998],PARAMETER["central_meridian",103.4279362360999],PARAMETER["false_easting",-14810.56199999999],PARAMETER["false_northing",8758.319999999991]]])WKT";
	const char* kMtI_Test5108_part_1_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Cassini_Soldner",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["latitude_of_origin",2.121679739999998],PARAMETER["central_meridian",103.4279362360999],PARAMETER["false_easting",-14810.56199999999],PARAMETER["false_northing",8758.319999999991]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5108Part1, Test5108_part_1)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62022");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5108Part1, Test5108_part_1_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5108_part_1_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5108Part1, Test5108_part_1_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(3377);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5108Part1, Test5108_part_1_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5108_part_1_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5108Part1, Test5108_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5108_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5108_part_1_MathTransform);
	ExecuteTests(d, i);
}
