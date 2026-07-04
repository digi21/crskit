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
            TestDirectTransform(d, Sexa2DecimalDegrees(80, 0, 00.952, CardinalPoint::N), Sexa2DecimalDegrees(63, 59, 57.636, CardinalPoint::W), 5422500.000, 18889800.002, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(40, 0, 01.109, CardinalPoint::N), Sexa2DecimalDegrees(63, 59, 59.050, CardinalPoint::W), 5158399.999, 14439199.987, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(0, 0, 01.113, CardinalPoint::N), Sexa2DecimalDegrees(63, 59, 58.627, CardinalPoint::W),  5054400.005, 10001999.999, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(19, 59, 59.742, CardinalPoint::S), Sexa2DecimalDegrees(64, 0, 01.683, CardinalPoint::W), 5081100.017, 7784599.993, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(60, 0, 01.443, CardinalPoint::S), Sexa2DecimalDegrees(63, 59, 59.573, CardinalPoint::W), 5276899.994, 3341099.995, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(40, 0, 00.774, CardinalPoint::S), Sexa2DecimalDegrees(70, 0, 00.752, CardinalPoint::W), 4645300.113, 5524200.123, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(39, 59, 58.609, CardinalPoint::S), Sexa2DecimalDegrees(67, 59, 58.320, CardinalPoint::W), 4816500.043, 5541700.028, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(39, 59, 58.522, CardinalPoint::S), Sexa2DecimalDegrees(65, 59, 58.748, CardinalPoint::W), 4987500.009, 5555200.001, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(39, 59, 58.893, CardinalPoint::S), Sexa2DecimalDegrees(63, 59, 58.920, CardinalPoint::W), 5158400.010, 5564799.987, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(40, 0, 00.649, CardinalPoint::S), Sexa2DecimalDegrees(62, 0, 00.280, CardinalPoint::W), 5329199.995, 5570500.009, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(40, 0, 01.190, CardinalPoint::S), Sexa2DecimalDegrees(60, 0, 00.000, CardinalPoint::W), 5500000.000, 5572399.996, 1E-2);
            TestDirectTransform(d, Sexa2DecimalDegrees(40, 0, 00.649, CardinalPoint::S), Sexa2DecimalDegrees(57, 59, 59.720, CardinalPoint::W), 5670800.005, 5570500.009, 1E-2);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(80, 0, 00.952, CardinalPoint::N), Sexa2DecimalDegrees(63, 59, 57.636, CardinalPoint::W));
        	}

	const char* kWkt_Test5101_part_4_Wkt = R"WKT(PROJCS["GIGS projCRS G11",GEOGCS["GIGS geogCRS G",DATUM["GIGS geodetic datum G",SPHEROID["GIGS ellipsoid F",6378137,298.257222169001,AUTHORITY["GIGS","67019"]],AUTHORITY["GIGS","66007"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64010"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",-89.99999999999992],PARAMETER["central_meridian",-59.99999999999994],PARAMETER["scale_factor",1],PARAMETER["false_easting",5500000],PARAMETER["false_northing",0],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["X", NORTH],AXIS["Y", EAST],AUTHORITY["GIGS","62018"]])WKT";
	const char* kWkt_Test5101_part_4_WktEpsg = R"WKT(PROJCS["POSGAR 98 / Argentina 5",GEOGCS["POSGAR 98",DATUM["Posiciones Geodesicas Argentinas 1998",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6190"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree (supplier to define representation)",0.01745329251994328,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4190"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",-89.99999999999992],PARAMETER["central_meridian",-59.99999999999994],PARAMETER["scale_factor",1],PARAMETER["false_easting",5500000],PARAMETER["false_northing",0],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314140356],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["X", NORTH],AXIS["Y", EAST],AUTHORITY["EPSG","22175"]])WKT";
	const char* kMtD_Test5101_part_4_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine", PARAMETER["num_row",3], PARAMETER["num_col", 3],PARAMETER["elt_0_0", 0],PARAMETER["elt_0_1", 1],PARAMETER["elt_0_2", 0],PARAMETER["elt_1_0", 1],PARAMETER["elt_1_1", 0],PARAMETER["elt_1_2", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 1]],PARAM_MT["Transverse_Mercator",PARAMETER["latitude_of_origin",-89.99999999999992],PARAMETER["central_meridian",-59.99999999999994],PARAMETER["scale_factor",1],PARAMETER["false_easting",5500000],PARAMETER["false_northing",0],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314140356]]])WKT";
	const char* kMtI_Test5101_part_4_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Transverse_Mercator",PARAMETER["latitude_of_origin",-89.99999999999992],PARAMETER["central_meridian",-59.99999999999994],PARAMETER["scale_factor",1],PARAMETER["false_easting",5500000],PARAMETER["false_northing",0],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314140356]]],PARAM_MT["Affine", PARAMETER["num_row",3], PARAMETER["num_col", 3],PARAMETER["elt_0_0", 0],PARAMETER["elt_0_1", 1],PARAMETER["elt_0_2", 0],PARAMETER["elt_1_0", 1],PARAMETER["elt_1_1", 0],PARAMETER["elt_1_2", 0],PARAMETER["elt_2_0", 0],PARAMETER["elt_2_1", 0],PARAMETER["elt_2_2", 1]]])WKT";
}

TEST(Test5101Part4, Test5101_part_4)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62018");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part4, Test5101_part_4_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5101_part_4_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part4, Test5101_part_4_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(22175);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part4, Test5101_part_4_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5101_part_4_WktEpsg);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5101Part4, Test5101_part_4_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5101_part_4_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5101_part_4_MathTransform);
	ExecuteTests(d, i);
}
