// The parameters of a projection built from the catalogue carry the names of the class parameters with the same EPSG
// code, whatever the order. Not part of the GIGS suite (no "Test5xxx" prefix).

#include <string>
#include <vector>

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	auto Names(std::shared_ptr<CoordinateSystems::IProjection> const& projection) -> std::vector<std::string>
	{
		std::vector<std::string> names;
		for (auto const& parameter : projection->GetParameters())
			names.push_back(parameter.GetName());
		return names;
	}
}

// EPSG 3856 (conversion of EPSG 3857) has 4 parameters; the class declared 5 with scale_factor third, so false
// easting landed in scale_factor and false northing in false_easting.
TEST(ProjectionParameters, PseudoMercatorMapsByCode)
{
	auto const projection = GetCoordinateSystemAuthorityFactory()->CreateProjection(3856, 1024);
	EXPECT_EQ((std::vector<std::string>{ "latitude_of_origin", "central_meridian", "false_easting", "false_northing" }), Names(projection));
}

TEST(ProjectionParameters, PseudoMercatorFalseOriginGoesToItsAxis)
{
	auto const transform = GetMathTransformFactory()->CreateParameterizedTransform("Popular_Visualisation_Pseudo_Mercator", {
		{ "semi_major", 6378137.0 }, { "semi_minor", 6356752.314245179 },
		{ "latitude_of_origin", 0.0 }, { "central_meridian", 0.0 }, { "false_easting", 1000.0 }, { "false_northing", 2000.0 } });
	auto const origin = transform->Transform({ 0.0, 0.0 });
	EXPECT_NEAR(1000.0, origin[0], 1e-6);
	EXPECT_NEAR(2000.0, origin[1], 1e-6);
}
// 9836 (Geocentric/topocentric conversions): the database gives the topocentric origin X, Y, Z; the class also takes
// the semi-axes, which come from the ellipsoid.
TEST(ProjectionParameters, TopocentricOriginMapsByCode)
{
	auto const projection = GetCoordinateSystemAuthorityFactory()->CreateProjection(15595, 9836);
	EXPECT_EQ((std::vector<std::string>{ "geocentric_x_of_topocentric_origin", "geocentric_y_of_topocentric_origin", "geocentric_z_of_topocentric_origin" }), Names(projection));
}