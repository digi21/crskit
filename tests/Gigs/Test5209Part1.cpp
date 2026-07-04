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
            TestDirectTransform(d, 1, 21200, 438129.87, 5827554.372, 1E-3);
            TestDirectTransform(d, 8001, 21200, 532099.1321, 5793352.358, 1E-3);
            TestDirectTransform(d, 1, 10000, 414188.46, 5761775.889, 1E-3);
            TestDirectTransform(d, 8001, 10000, 508157.7221, 5727573.875, 1E-3);

            TestInverseTransform(i, 5406.044399, 20442.91044, 500000, 5800000, 1E-3);
            TestInverseTransform(i, -2111.496567, 14970.58815, 400000, 5800000, 1E-3);
            TestInverseTransform(i, 4383.435063, 2671.667361, 450000, 5700000, 1E-3);
            TestInverseTransform(i, 3360.825726, -15099.57572, 400000, 5600000, 1E-3);

            ExecuteIterations(d, i, 1, 21200);
        	}

	const char* kMtD_Test5209_part_1_Wkt = R"WKT(PARAM_MT["p6",PARAMETER["bin_grid_origin_i",1],PARAMETER["bin_grid_origin_j",10000],PARAMETER["bin_grid_origin_easting",414188.46],PARAMETER["bin_grid_origin_northing",5761775.889],PARAMETER["scale_factor",1],PARAMETER["bin_width_of_i_axis",25],PARAMETER["bin_width_of_j_axis",12.5],PARAMETER["map_grid_bearing_of_bin_grid_j_axis",20],PARAMETER["bin_node_increment_on_i_axis",2],PARAMETER["bin_node_increment_on_j_axis",2]])WKT";
	const char* kMtI_Test5209_part_1_Wkt = R"WKT(INVERSE_MT[PARAM_MT["p6",PARAMETER["bin_grid_origin_i",1],PARAMETER["bin_grid_origin_j",10000],PARAMETER["bin_grid_origin_easting",414188.46],PARAMETER["bin_grid_origin_northing",5761775.889],PARAMETER["scale_factor",1],PARAMETER["bin_width_of_i_axis",25],PARAMETER["bin_width_of_j_axis",12.5],PARAMETER["map_grid_bearing_of_bin_grid_j_axis",20],PARAMETER["bin_node_increment_on_i_axis",2],PARAMETER["bin_node_increment_on_j_axis",2]]])WKT";
}

TEST(Test5209Part1, DISABLED_Test5209_part_1_Epsg)
{
	// Not applicable: the GIGS 5209 bin grid is a user-defined conversion with no
	// equivalent operation in the EPSG dataset. EPSG does define P6 seismic bin grid
	// methods (9666 "I=J+90", 1049 "I=J-90") and example operations (6918, 6919), but
	// their parameters differ from the GIGS reference (origin, scale factor and node
	// increments), so they would not reproduce the GIGS 5209 test points. Hence no
	// _Epsg/_WktEpsg authority-factory variant is constructible; the user-defined
	// conversion is fully covered by Test5209_part_1_Wkt (EPSG dataset / GIGS design
	// limit, not a pending failure).
	GTEST_SKIP() << "Not applicable: GIGS 5209 is a user-defined bin grid with no equivalent EPSG operation (EPSG bin grid examples 6918/6919 use different parameters); covered by the _Wkt variant.";
}

TEST(Test5209Part1, Test5209_part_1_Wkt)
{
	auto const mtf = GetMathTransformFactory();
	auto const d = mtf->CreateFromWkt(kMtD_Test5209_part_1_Wkt);
	auto const i = mtf->CreateFromWkt(kMtI_Test5209_part_1_Wkt);
	ExecuteTests(d, i);
}
