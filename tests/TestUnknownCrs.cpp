// Verifies the unknown/local CRS handling: by default a transform between a known and an
// unknown/local system is rejected; UnknownCrsPolicy::Identity places it as-is; and the
// resolveTransform delegate lets the caller decide (e.g. show a dialog).

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	const char* kKnownGeographic = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	const char* kLocal = R"WKT(LOCAL_CS["Local",LOCAL_DATUM["Local",32767],UNIT["m",1.0],AXIS["X", EAST],AXIS["Y", NORTH]])WKT";

	auto Known() { return GetCoordinateSystemFactory()->CreateFromWkt(kKnownGeographic); }
	auto Local() { return GetCoordinateSystemFactory()->CreateFromWkt(kLocal); }

	// Compounds sharing the same horizontal (so it resolves to identity by WKT equality, no EPSG)
	// with, respectively, a known and an unknown/local vertical -> exercises the vertical resolve path.
	std::string KnownVerticalCompound() { return std::string("COMPD_CS[\"known\",") + kKnownGeographic + R"(,VERT_CS["H",VERT_DATUM["H",2005],UNIT["m",1.0],AXIS["Up", UP]]])"; }
	std::string LocalVerticalCompound() { return std::string("COMPD_CS[\"localv\",") + kKnownGeographic + R"(,LOCAL_CS["LocalV",LOCAL_DATUM["LocalV",32767],UNIT["m",1.0],AXIS["Z", UP]]])"; }
}

TEST(UnknownCrs, RejectByDefaultThrows)
{
	EXPECT_THROW(static_cast<void>(GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Known(), Local())), std::exception);
}

TEST(UnknownCrs, IdentityPolicyDoesNotThrow)
{
	CoordinateTransformationOptions options;
	options.unknownCrsPolicy = UnknownCrsPolicy::Identity;

	auto const ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Known(), Local(), options);
	ASSERT_NE(nullptr, ct);
	ASSERT_NE(nullptr, ct->GetMathTransform());
}

TEST(UnknownCrs, DelegateReturningNullRejects)
{
	CoordinateTransformationOptions options;
	options.resolveTransform = [](auto const&, auto const&) { return std::shared_ptr<IMathTransform>{}; };

	EXPECT_THROW(static_cast<void>(GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Known(), Local(), options)), std::exception);
}

TEST(UnknownCrs, DelegateProvidesTheTransform)
{
	bool called = false;
	auto const provided = GetMathTransformFactory()->CreateFromWkt(R"WKT(PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",1],PARAMETER["elt_0_1",0],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",0],PARAMETER["elt_1_1",1],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]])WKT");

	CoordinateTransformationOptions options;
	options.resolveTransform = [&](auto const&, auto const&) { called = true; return provided; };

	auto const ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Known(), Local(), options);
	EXPECT_TRUE(called);
	ASSERT_NE(nullptr, ct);
	EXPECT_EQ(provided, ct->GetMathTransform());
}

TEST(UnknownCrs, CompoundUnknownVerticalRejectThrows)
{
	auto const source = GetCoordinateSystemFactory()->CreateFromWkt(KnownVerticalCompound());
	auto const target = GetCoordinateSystemFactory()->CreateFromWkt(LocalVerticalCompound());

	EXPECT_THROW(static_cast<void>(GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)), std::exception);
}

TEST(UnknownCrs, CompoundUnknownVerticalIdentityDoesNotThrow)
{
	auto const source = GetCoordinateSystemFactory()->CreateFromWkt(KnownVerticalCompound());
	auto const target = GetCoordinateSystemFactory()->CreateFromWkt(LocalVerticalCompound());

	CoordinateTransformationOptions options;
	options.unknownCrsPolicy = UnknownCrsPolicy::Identity;

	auto const ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target, options);
	ASSERT_NE(nullptr, ct);
	ASSERT_NE(nullptr, ct->GetMathTransform());
}
