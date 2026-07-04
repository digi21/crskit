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
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 778711.230, 16661953.040, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(20, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 1023538.687, 12219308.238, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(20, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 1023538.687, 7780691.762, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(40, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 926893.302, 5560253.083, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 596813.055, 1114251.292, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(136, 0, 00.000, CardinalPoint::E), 221288.770, 3338046.960, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(138, 0, 00.000, CardinalPoint::E), 332705.179, 3344794.516, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(140, 0, 00.000, CardinalPoint::E), 444223.733, 3348167.265, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(142, 0, 00.000, CardinalPoint::E), 555776.267, 3348167.265, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(144, 0, 00.000, CardinalPoint::E), 667294.821, 3344794.516, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 778711.230, 3338046.960, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(148, 0, 00.000, CardinalPoint::E), 889956.701, 3327920.506, 1E-2);

            TestInverseTransform(i, Sexa2DecimalDegrees(80, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 596813.055, 18885748.708, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(40, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 926893.302, 14439746.917, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(0, 0, 00.000, CardinalPoint::N), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 1057087.120, 10000000.000, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(146, 0, 00.000, CardinalPoint::E), 778711.230, 3338046.960, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(137, 0, 00.000, CardinalPoint::E), 276979.926, 3341842.798, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(139, 0, 00.000, CardinalPoint::E), 388455.958, 3346902.565, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(141, 0, 00.000, CardinalPoint::E), 500000.000, 3348588.810, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(143, 0, 00.000, CardinalPoint::E), 611544.042, 3346902.565, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(145, 0, 00.000, CardinalPoint::E), 723020.074, 3341842.798, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(147, 0, 00.000, CardinalPoint::E), 834359.668, 3333406.428, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::S), Sexa2DecimalDegrees(149, 0, 00.000, CardinalPoint::E), 945493.565, 3321588.377, 1E-6);
	}

	const char* kWkt_Test5101_part_3_Wkt = R"WKT(PROJCS["GIGS projCRS F7",GEOGCS["GIGS geogCRS F",DATUM["GIGS geodetic datum F",SPHEROID["GIGS ellipsoid F",6378137,298.257222169001,AUTHORITY["GIGS","67019"]],AUTHORITY["GIGS","66006"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64009"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",140.9999999999999],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",10000000],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["E", EAST],AXIS["N", NORTH],AUTHORITY["GIGS","62014"]])WKT";
	const char* kWkt_Test5101_part_3_WktEpsg = R"WKT(PROJCS["GDA94 / MGA zone 54",GEOGCS["GDA94",DATUM["Geocentric Datum of Australia 1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4283"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",140.9999999999999],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",10000000],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314140356],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["E", EAST],AXIS["N", NORTH],AUTHORITY["EPSG","28354"]])WKT";
	const char* kMtD_Test5101_part_3_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Transverse_Mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",140.9999999999999],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",10000000]]])WKT";
	const char* kMtI_Test5101_part_3_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Transverse_Mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",140.9999999999999],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",10000000]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5101Part3, Test5101_part_3)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62014");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part3, Test5101_part_3_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5101_part_3_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part3, Test5101_part_3_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(28354);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part3, Test5101_part_3_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5101_part_3_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part3, Test5101_part_3_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5101_part_3_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5101_part_3_MathTransform);
	ExecuteTests(d, i);
}
