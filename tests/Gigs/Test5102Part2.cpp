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
            TestDirectTransform(d, 64.44444444, 2.958634256, 760724.023, 3457334.864, 1E-3);
            TestDirectTransform(d, 62.22222222, 2.958634256, 768397.648, 3230915.06, 1E-3);
            TestDirectTransform(d, 60, 2.958634256, 776020.989, 3005978.979, 1E-3);
            TestDirectTransform(d, 56.66666667, 1.847523144, 717027.602, 2668679.866, 1E-3);
            TestDirectTransform(d, 54.44444444, 1.847523144, 721740.59, 2445932.319, 1E-3);
            TestDirectTransform(d, 58.88888889, 0.736412033, 644765.081, 2891102.088, 1E-3);
            TestDirectTransform(d, 58.88888889, 2.958634256, 779817.454, 2893959.584, 1E-3);
            TestDirectTransform(d, 58.88888889, 5.180856478, 914753.403, 2900252.301, 1E-3);
            TestDirectTransform(d, 58.88888889, 7.4030787, 1049485.565, 2909976.163, 1E-3);
            TestDirectTransform(d, 58.88888889, 9.625300922, 1183926.705, 2923124.876, 1E-3);

            TestInverseTransform(i, 63.33333333, 2.958634256, 764567.882, 3343917.044, 1E-6);
            TestInverseTransform(i, 61.11111111, 2.958634256, 772214.859, 3118283.535, 1E-6);
            TestInverseTransform(i, 58.88888889, 2.958634256, 779817.454, 2893959.584, 1E-6);
            TestInverseTransform(i, 55.55555556, 1.847523144, 719385.487, 2557240.347, 1E-6);
            TestInverseTransform(i, 52, 1.847523144, 726915.726, 2201342.518, 1E-6);
            TestInverseTransform(i, 58.88888889, 1.847523144, 712300.356, 2892101.266, 1E-6);
            TestInverseTransform(i, 58.88888889, 4.069745367, 847305.444, 2896676.742, 1E-6);
            TestInverseTransform(i, 58.88888889, 6.291967589, 982150.413, 2904685.68, 1E-6);
            TestInverseTransform(i, 58.88888889, 8.514189811, 1116747.958, 2916122.894, 1E-6);

            ExecuteIterations(d, i, 64.44444444, 2.958634256);
        	}

	const char* kWkt_Test5102_part_2_Wkt = R"WKT(PROJCS["GIGS projCRS H19",GEOGCS["GIGS geogCRS H",DATUM["GIGS geodetic datum H",SPHEROID["GIGS ellipsoid H",6378249.2,293.4660212936269,AUTHORITY["GIGS","67011"]],AUTHORITY["GIGS","66008"]],PRIMEM["GIGS PM H",2.33722917,AUTHORITY["GIGS","68903"]],UNIT["GIGS unit A4 (grad)",0.01570796326794895,AUTHORITY["GIGS","69105"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["GIGS","64011"]],PROJECTION["Lambert_Conformal_Conic_1SP"],PARAMETER["latitude_of_origin",46.79999999999995],PARAMETER["central_meridian",0],PARAMETER["scale_factor",0.99987742],PARAMETER["false_easting",600000],PARAMETER["false_northing",2200000],PARAMETER["semi_major",6378249.2],PARAMETER["semi_minor",6356515],UNIT["GIGS unit L0 (metre)",1,AUTHORITY["GIGS","69001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["GIGS","62026"]])WKT";
	const char* kMtD_Test5102_part_2_MathTransform = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0.8999999999999991],PARAMETER["elt_0_1",0],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",0],PARAMETER["elt_1_1",0.8999999999999991],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",0.8999999999999991]],PARAM_MT["Lambert_Conformal_Conic_1SP",PARAMETER["semi_major",6378249.2],PARAMETER["semi_minor",6356515],PARAMETER["latitude_of_origin",46.79999999999995],PARAMETER["central_meridian",0],PARAMETER["scale_factor",0.99987742],PARAMETER["false_easting",600000],PARAMETER["false_northing",2200000]]])WKT";
	const char* kMtI_Test5102_part_2_MathTransform = R"WKT(CONCAT_MT[INVERSE_MT[PARAM_MT["Lambert_Conformal_Conic_1SP",PARAMETER["semi_major",6378249.2],PARAMETER["semi_minor",6356515],PARAMETER["latitude_of_origin",46.79999999999995],PARAMETER["central_meridian",0],PARAMETER["scale_factor",0.99987742],PARAMETER["false_easting",600000],PARAMETER["false_northing",2200000]]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",1.111111111111112],PARAMETER["elt_0_1",0],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",0],PARAMETER["elt_1_1",1.111111111111112],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1.111111111111112]],PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]]])WKT";

	const char* kWktEpsg_Test5102_part_2 = R"WKT(PROJCS["NTF (Paris) / Lambert zone II",GEOGCS["NTF (Paris)",DATUM["Nouvelle Triangulation Francaise (Paris)",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269,AUTHORITY["EPSG","7011"]],AUTHORITY["EPSG","6807"]],PRIMEM["Paris",2.5969213,AUTHORITY["EPSG","8903"]],UNIT["grad",0.01570796326794895,AUTHORITY["EPSG","9105"]],AXIS["Lat", NORTH],AXIS["Long", EAST],AUTHORITY["EPSG","4807"]],PROJECTION["Lambert_Conformal_Conic_1SP"],PARAMETER["latitude_of_origin",46.79999999999995],PARAMETER["central_meridian",0],PARAMETER["scale_factor",0.99987742],PARAMETER["false_easting",600000],PARAMETER["false_northing",2200000],PARAMETER["semi_major",6378249.2],PARAMETER["semi_minor",6356515],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["X", EAST],AXIS["Y", NORTH],AUTHORITY["EPSG","27572"]])WKT";
}

TEST(Test5102Part2, Test5102_part_2)
{
	Gigs::GigsFactory gigs;
	auto const pcs = gigs.CreateProjectedCoordinateSystem("62026");
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5102Part2, Test5102_part_2_Wkt)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt_Test5102_part_2_Wkt);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5102Part2, Test5102_part_2_Epsg)
{
	auto const pcs = GetCoordinateSystemAuthorityFactory()->CreateProjectedCoordinateSystem(27572);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5102Part2, Test5102_part_2_WktEpsg)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWktEpsg_Test5102_part_2);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);
	auto const gcs = pcs->GetGeographicCoordinateSystem();
	auto const [d, i] = BuildDirectInverse(gcs, pcs);
	ExecuteTests(d, i);
}

TEST(Test5102Part2, Test5102_part_2_MathTransform)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5102_part_2_MathTransform);
	auto const i = mtf->CreateFromWkt(kMtI_Test5102_part_2_MathTransform);
	ExecuteTests(d, i);
}
