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
            TestDirectTransform(d, Sexa2DecimalDegrees(0, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(54, 0, 0, CardinalPoint::W), 5000000.00, 10000000.00, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::S), Sexa2DecimalDegrees(45, 0, 0, CardinalPoint::W), 5996378.71, 9328349.944, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(13, 0, 0, CardinalPoint::S), Sexa2DecimalDegrees(41, 0, 0, CardinalPoint::W), 6409689.587, 8526306.262, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(24, 0, 0, CardinalPoint::S), Sexa2DecimalDegrees(37, 0, 0, CardinalPoint::W), 6725584.492, 7240461.996, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(30, 0, 0, CardinalPoint::S), Sexa2DecimalDegrees(57, 0, 0, CardinalPoint::W), 4710574.223, 6676097.811, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(27, 30, 0, CardinalPoint::S), Sexa2DecimalDegrees(47, 0, 0, CardinalPoint::W), 5691318.147, 6937461.051, 1E-3);
            TestDirectTransform(d, Sexa2DecimalDegrees(22, 30, 0, CardinalPoint::S), Sexa2DecimalDegrees(30, 0, 0, CardinalPoint::W), 7458947.701, 7313327.317, 1E-3);

            TestInverseTransform(i, Sexa2DecimalDegrees(6, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(45, 0, 0, CardinalPoint::W), 5996378.71, 10671650.06, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(0, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(45, 0, 0, CardinalPoint::W), 6001875.417, 10000000, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(20, 0, 0, CardinalPoint::S), Sexa2DecimalDegrees(38, 0, 0, CardinalPoint::W), 6671808.92, 7707735.73, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(30, 0, 0, CardinalPoint::S), Sexa2DecimalDegrees(36, 0, 0, CardinalPoint::W), 6729619.74, 6543762.576, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(29, 22, 02.916, CardinalPoint::S), Sexa2DecimalDegrees(54, 0, 00.000, CardinalPoint::W), 5000000, 6750000, 1E-6);
            TestInverseTransform(i, Sexa2DecimalDegrees(24, 0, 0, CardinalPoint::S), Sexa2DecimalDegrees(37, 0, 0, CardinalPoint::W), 6725584.492, 7240461.996, 1E-6);

            ExecuteIterations(d, i, Sexa2DecimalDegrees(0, 0, 0, CardinalPoint::N), Sexa2DecimalDegrees(54, 0, 0, CardinalPoint::W));
        	}

	const char* kWkt_Test5107_part_1_Wkt = R"WKT(PROJCS["GIGS projCRS G12",GEOGCS["GIGS geogCRS G",DATUM["GIGS geodetic datum G",SPHEROID["GIGS ellipsoid F",6378137,298.257222169001,AUTHORITY["GIGS","67019"]],AUTHORITY["GIGS","66007"]],PRIMEM["GIGS PM A",0,AUTHORITY["GIGS","68901"]],UNIT["GIGS unit A2 (degree)",0.01745329251994328,AUTHORITY["GIGS","69102"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64010"]],PROJECTION["polyconic"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",-53.99999999999995],PARAMETER["false_easting",5000000],PARAMETER["false_northing",10000000],PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["GIGS","62019"]])WKT";
	const char* kMtD_Test5107_part_1_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["polyconic",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",-53.99999999999995],PARAMETER["false_easting",5000000],PARAMETER["false_northing",10000000]]])WKT";
	const char* kMtI_Test5107_part_1_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["polyconic",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314145231],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",-53.99999999999995],PARAMETER["false_easting",5000000],PARAMETER["false_northing",10000000]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";
}

TEST(Test5107Part1, Test5107_part_1)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62019");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5107Part1, Test5107_part_1_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5107_part_1_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5107Part1, Test5107_part_1_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5107_part_1_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5107_part_1_MathTransform);
	ExecuteTests(d, i);
}
