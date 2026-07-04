// A CoordinateTransformation must report the EPSG coordinate-operation code it was built from
// (GetAuthorityCode()), not a hardcoded "0". Synthetic transformations (axis/unit changes, composed
// projections, user-resolved) carry no code: GetAuthority() is empty and GetAuthorityCode() is "0".

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	auto Crs(char const* wkt) { return GetCoordinateSystemFactory()->CreateFromWkt(wkt); }

	char const* kWgs84 = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	char const* kWgs84SwappedAxes = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Long", EAST],AXIS["Lat", NORTH]])WKT";
}

// Built through the real EPSG path (OSGB36 4277 -> WGS 84 4326, selecting coordinate operation 1314,
// the same one Test5203 exercises). The resulting transformation must report that operation as its
// authority code instead of the former hardcoded "0".
TEST(TransformationAuthorityCode, EpsgTransformReportsItsCode)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4277);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326);

	DelegateDetectedMultipleCoordinateOperations const selectOperation =
		[](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) -> int { return 1314; };

	auto const ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		std::dynamic_pointer_cast<CoordinateSystem>(source),
		std::dynamic_pointer_cast<CoordinateSystem>(target),
		CoordinateTransformationOptions{ .selectOperation = selectOperation });

	ASSERT_NE(nullptr, ct);
	EXPECT_EQ("EPSG", ct->GetAuthority());
	EXPECT_EQ("1314", ct->GetAuthorityCode());
	// OSGB36 -> WGS 84 is a change of geodetic datum.
	EXPECT_EQ(TransformType::Transformation, ct->GetTransformType());
}

// A purely synthetic transformation (here just an axis swap between the same CRS) corresponds to no
// EPSG operation: no authority and the "no code" sentinel.
TEST(TransformationAuthorityCode, SyntheticTransformHasNoCode)
{
	auto const ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(kWgs84), Crs(kWgs84SwappedAxes));
	ASSERT_NE(nullptr, ct);
	EXPECT_EQ("", ct->GetAuthority());
	// No EPSG operation: GetAuthorityCode() is empty to stay consistent with GetAuthority().
	EXPECT_EQ("", ct->GetAuthorityCode());
	// Same datum, only the axis order changes: a Conversion, not a datum Transformation.
	EXPECT_EQ(TransformType::Conversion, ct->GetTransformType());
}
