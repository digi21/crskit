// TOWGS84 (Bursa-Wolf parameters to WGS 84) must survive a WKT round-trip: parsed from the DATUM
// node into the HorizontalDatum, and emitted back by GetWkt() in OGC WKT 1 order.

#include "TestBase.h"
#include "../crskit/CoordinateSystems/HorizontalDatum.h"
#include "../crskit/CoordinateSystems/IHorizontalCoordinateSystem.h"
#include "../crskit/OpenGisException.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace TestCrsKit;

namespace
{
	auto Crs(char const* wkt) { return GetCoordinateSystemFactory()->CreateFromWkt(wkt); }

	char const* kWithTowgs84 = R"WKT(GEOGCS["Test datum with shift",DATUM["Test_Datum",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,1.5,2.5,3.5,7]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";

	std::shared_ptr<HorizontalDatum> DatumOf(char const* wkt)
	{
		auto const cs = std::dynamic_pointer_cast<IHorizontalCoordinateSystem>(Crs(wkt));
		return cs ? std::dynamic_pointer_cast<HorizontalDatum>(cs->GetHorizontalDatum()) : nullptr;
	}
}

TEST(Towgs84, ParsedFromWkt)
{
	auto const datum = DatumOf(kWithTowgs84);
	ASSERT_NE(nullptr, datum);

	auto const towgs84 = datum->GetWgs84ConversionInfo();
	ASSERT_NE(nullptr, towgs84);
	EXPECT_DOUBLE_EQ(-87.0, towgs84->GetDx());
	EXPECT_DOUBLE_EQ(-98.0, towgs84->GetDy());
	EXPECT_DOUBLE_EQ(-121.0, towgs84->GetDz());
	EXPECT_DOUBLE_EQ(1.5, towgs84->GetEx());
	EXPECT_DOUBLE_EQ(2.5, towgs84->GetEy());
	EXPECT_DOUBLE_EQ(3.5, towgs84->GetEz());
	EXPECT_DOUBLE_EQ(7.0, towgs84->GetPpm());
}

TEST(Towgs84, RoundTripsThroughWkt)
{
	auto const datum = DatumOf(kWithTowgs84);
	ASSERT_NE(nullptr, datum);

	// Emitted in OGC WKT 1 order, with the seven parameters intact.
	EXPECT_NE(std::string::npos, datum->GetWkt().find("TOWGS84[-87,-98,-121,1.5,2.5,3.5,7]"));
}

TEST(Towgs84, AbsentWhenNotInWkt)
{
	char const* noShift = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	auto const datum = DatumOf(noShift);
	ASSERT_NE(nullptr, datum);
	EXPECT_EQ(nullptr, datum->GetWgs84ConversionInfo());
	EXPECT_EQ(std::string::npos, datum->GetWkt().find("TOWGS84"));
}

// A geographic CRS that carries TOWGS84 but no EPSG identity transforms to WGS 84 using those
// parameters. Cross-checked against the trusted EPSG path: TOWGS84[-87,-98,-121,0,0,0,0] is exactly
// the parameter set of EPSG operation 1133 ("ED50 to WGS 84 (1)"), so both routes must agree.
TEST(Towgs84Transform, MatchesEpsgOperation)
{
	char const* ed50 = R"WKT(GEOGCS["ED50",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	char const* wgs84 = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";

	auto const towgs84Ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(ed50), Crs(wgs84));
	ASSERT_NE(nullptr, towgs84Ct);
	// Built from embedded parameters, not an EPSG operation: synthetic, no authority code.
	EXPECT_EQ("", towgs84Ct->GetAuthorityCode());

	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4230);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326);
	auto const epsg = BuildDirectInverseWithOperation(source, target, 1133);

	auto const viaTowgs84 = towgs84Ct->GetMathTransform()->Transform({ 40.0, -3.0 });
	auto const viaEpsg = epsg.d->Transform({ 40.0, -3.0 });
	EXPECT_NEAR(viaEpsg[0], viaTowgs84[0], 1e-7);
	EXPECT_NEAR(viaEpsg[1], viaTowgs84[1], 1e-7);
}

// Safety rule: if a datum neither carries TOWGS84 nor is WGS 84, its relation to WGS 84 is unknown.
// The library must refuse rather than silently assume the identity (which would be wrong coordinates).
TEST(Towgs84Transform, RefusesWhenDatumRelationUnknown)
{
	char const* ed50NoShift = R"WKT(GEOGCS["My ED50",DATUM["My_European_Datum_1950",SPHEROID["International 1924",6378388,297]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	char const* nad27NoShift = R"WKT(GEOGCS["My NAD27",DATUM["My_North_American_Datum_1927",SPHEROID["Clarke 1866",6378206.4,294.9786982138982]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";

	EXPECT_THROW(
		static_cast<void>(GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(ed50NoShift), Crs(nad27NoShift))),
		TransformationNotFoundException);
}

namespace
{
	// ED50 / UTM zone 30N carrying the same TOWGS84 as EPSG operation 1133 (ED50 -> WGS 84 (1)).
	// The datum carries TOWGS84, so ProjectedCoordinateSystem::FromWkt keeps the WKT definition instead
	// of snapping it to the same-named catalogue entry (which would drop the Bursa-Wolf parameters).
	char const* kEd50Utm30nWithTowgs84 = R"WKT(PROJCS["ED50 / UTM zone 30N",GEOGCS["ED50",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",-3],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],UNIT["metre",1],AXIS["X",EAST],AXIS["Y",NORTH]])WKT";

	// The geographic ED50 underlying the projected CRS above (same datum, same TOWGS84).
	char const* kEd50GeoWithTowgs84 = R"WKT(GEOGCS["ED50",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";

	char const* kWgs84Geo = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";

	// WGS 84 / UTM zone 30N (no datum shift: the datum is WGS 84 itself).
	char const* kWgs84Utm30n = R"WKT(PROJCS["WGS 84 / UTM zone 30N",GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",-3],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],UNIT["metre",1],AXIS["X",EAST],AXIS["Y",NORTH]])WKT";
}

// A projected CRS that carries TOWGS84 transforms to WGS 84 by un-projecting, shifting the datum
// through WGS 84 with the Bursa-Wolf parameters, and (here) staying geographic on the target side.
// Cross-checked against composing two independently trusted steps: the same-datum un-projection and
// the geographic TOWGS84 shift (the latter anchored to EPSG operation 1133 by MatchesEpsgOperation).
TEST(Towgs84ProjectedTransform, ProjectedSourceMatchesUnprojectThenShift)
{
	auto const mine = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(kEd50Utm30nWithTowgs84), Crs(kWgs84Geo))->GetMathTransform();

	// Reference path: un-project (same datum, no shift) then apply the geographic datum shift.
	auto const unproject = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(kEd50Utm30nWithTowgs84), Crs(kEd50GeoWithTowgs84))->GetMathTransform();
	auto const shift = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(kEd50GeoWithTowgs84), Crs(kWgs84Geo))->GetMathTransform();

	// A point inside zone 30N (easting, northing in metres).
	std::vector<double> const projected{ 500000.0, 4400000.0 };

	auto const viaMine = mine->Transform(projected);
	auto const viaReference = shift->Transform(unproject->Transform(projected));

	ASSERT_EQ(2u, viaMine.size());
	EXPECT_NEAR(viaReference[0], viaMine[0], 1e-9);
	EXPECT_NEAR(viaReference[1], viaMine[1], 1e-9);
}

// Projected <-> projected across a datum change: ED50 / UTM 30N (with TOWGS84) <-> WGS 84 / UTM 30N.
// The round trip must return to the start, and the forward step must move the point appreciably
// (the ~180 m ED50->WGS 84 translation), proving the datum shift is actually applied (not the identity).
TEST(Towgs84ProjectedTransform, ProjectedToProjectedRoundTrips)
{
	auto const ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(kEd50Utm30nWithTowgs84), Crs(kWgs84Utm30n));
	auto const d = ct->GetMathTransform();
	auto const i = d->GetInverse();

	std::vector<double> const start{ 500000.0, 4400000.0 };

	auto const forward = d->Transform(start);
	ExpectFinite(forward[0]);
	ExpectFinite(forward[1]);

	// The datum shift moved the point: ED50 and WGS 84 differ by ~180 m, so the planimetric shift is
	// well above a metre (and far from zero, which is what an erroneous identity would give).
	auto const shiftMagnitude = std::hypot(forward[0] - start[0], forward[1] - start[1]);
	EXPECT_GT(shiftMagnitude, 1.0);

	// Round trip back to the start. The tolerance is at the centimetre level because the chain goes
	// through a Transverse Mercator inverse, a geocentric Bursa-Wolf shift and a forward projection
	// (and the reverse), each contributing sub-millimetre numerical error.
	auto const back = i->Transform(forward);
	EXPECT_NEAR(start[0], back[0], 1e-2);
	EXPECT_NEAR(start[1], back[1], 1e-2);
}

namespace
{
	std::shared_ptr<HorizontalDatum> DatumOfCode(int code)
	{
		auto const cs = std::dynamic_pointer_cast<IHorizontalCoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code));
		return cs ? std::dynamic_pointer_cast<HorizontalDatum>(cs->GetHorizontalDatum()) : nullptr;
	}
}

// A CRS loaded from the EPSG catalogue emits TOWGS84 derived from its most accurate transformation to
// WGS 84 that is expressible as a 7-parameter Helmert. ED50 (4230): the best such operation is EPSG 1311
// "ED50 to WGS 84 (18)", a Position Vector (method 9606) with non-zero rotation and scale.
TEST(Towgs84FromEpsg, EmittedForDatumWithShift)
{
	auto const datum = DatumOfCode(4230);
	ASSERT_NE(nullptr, datum);

	auto const towgs84 = datum->GetWgs84ConversionInfo();
	ASSERT_NE(nullptr, towgs84);
	EXPECT_NEAR(-89.5, towgs84->GetDx(), 1e-9);
	EXPECT_NEAR(-93.8, towgs84->GetDy(), 1e-9);
	EXPECT_NEAR(-123.1, towgs84->GetDz(), 1e-9);
	EXPECT_NEAR(0.0, towgs84->GetEx(), 1e-9);
	EXPECT_NEAR(0.0, towgs84->GetEy(), 1e-9);
	EXPECT_NEAR(-0.156, towgs84->GetEz(), 1e-9);
	EXPECT_NEAR(1.2, towgs84->GetPpm(), 1e-9);

	// And it is actually written out in the datum's WKT.
	EXPECT_NE(std::string::npos, datum->GetWkt().find("TOWGS84["));
}

// Coordinate Frame (method 9607) operations differ from Position Vector only in the sign of the three
// rotations, so the emitted TOWGS84 (always Position Vector) negates them. MGI (4312): the best
// expressible operation is EPSG 1194, a Coordinate Frame whose stored rotations are -4.7354, -1.3145,
// -5.393; the datum must therefore report +4.7354, +1.3145, +5.393.
TEST(Towgs84FromEpsg, CoordinateFrameNegatedToPositionVector)
{
	auto const datum = DatumOfCode(4312);
	ASSERT_NE(nullptr, datum);

	auto const towgs84 = datum->GetWgs84ConversionInfo();
	ASSERT_NE(nullptr, towgs84);
	EXPECT_NEAR(601.705, towgs84->GetDx(), 1e-9);
	EXPECT_NEAR(84.263, towgs84->GetDy(), 1e-9);
	EXPECT_NEAR(485.227, towgs84->GetDz(), 1e-9);
	EXPECT_NEAR(4.7354, towgs84->GetEx(), 1e-9);
	EXPECT_NEAR(1.3145, towgs84->GetEy(), 1e-9);
	EXPECT_NEAR(5.393, towgs84->GetEz(), 1e-9);
	EXPECT_NEAR(-2.3887, towgs84->GetPpm(), 1e-9);
}

// WGS 84 itself has no shift to WGS 84, so no TOWGS84 is emitted.
TEST(Towgs84FromEpsg, AbsentForWgs84)
{
	auto const datum = DatumOfCode(4326);
	ASSERT_NE(nullptr, datum);
	EXPECT_EQ(nullptr, datum->GetWgs84ConversionInfo());
	EXPECT_EQ(std::string::npos, datum->GetWkt().find("TOWGS84"));
}

// End-to-end check with NON-ZERO rotations, which exercises the arc-seconds <-> degrees conversion in
// the transformation engine (the earlier transform tests all used zero rotations). A WKT MGI datum
// carrying the Position Vector TOWGS84 of EPSG 1194 (Bessel 1841, same as MGI) must transform to WGS 84
// exactly like the trusted EPSG path 4312 -> 4326 through operation 1194 (a Coordinate Frame, which is
// the same shift as the negated-rotation Position Vector we carry).
TEST(Towgs84Transform, MatchesEpsgOperationWithRotations)
{
	char const* mgi = R"WKT(GEOGCS["MGI",DATUM["Militar_Geographische_Institut",SPHEROID["Bessel 1841",6377397.155,299.1528128],TOWGS84[601.705,84.263,485.227,4.7354,1.3145,5.393,-2.3887]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	char const* wgs84 = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";

	auto const towgs84Ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(mgi), Crs(wgs84));
	ASSERT_NE(nullptr, towgs84Ct);

	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4312);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326);
	auto const epsg = BuildDirectInverseWithOperation(source, target, 1194);

	// A point in Austria (latitude, longitude).
	auto const viaTowgs84 = towgs84Ct->GetMathTransform()->Transform({ 47.5, 14.0 });
	auto const viaEpsg = epsg.d->Transform({ 47.5, 14.0 });
	EXPECT_NEAR(viaEpsg[0], viaTowgs84[0], 1e-7);
	EXPECT_NEAR(viaEpsg[1], viaTowgs84[1], 1e-7);
}

// The Coordinate Frame convention (EPSG 9607, ESRI) can be selected per call. A WKT MGI carrying the
// RAW parameters of operation 1194 (a Coordinate Frame, rotations -4.7354, -1.3145, -5.393) interpreted
// with CoordinateFrame must reproduce that EPSG operation directly -- whereas the default (Position
// Vector) would read the same numbers with the wrong rotation sign and land elsewhere.
TEST(Towgs84Transform, CoordinateFrameConventionMatchesEpsgCoordinateFrameOperation)
{
	char const* mgiCoordinateFrame = R"WKT(GEOGCS["MGI",DATUM["Militar_Geographische_Institut",SPHEROID["Bessel 1841",6377397.155,299.1528128],TOWGS84[601.705,84.263,485.227,-4.7354,-1.3145,-5.393,-2.3887]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	char const* wgs84 = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";

	CoordinateTransformationOptions const options{ .towgs84RotationConvention = Towgs84RotationConvention::CoordinateFrame };
	auto const towgs84Ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(mgiCoordinateFrame), Crs(wgs84), options);
	ASSERT_NE(nullptr, towgs84Ct);

	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4312);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326);
	auto const epsg = BuildDirectInverseWithOperation(source, target, 1194);

	auto const viaTowgs84 = towgs84Ct->GetMathTransform()->Transform({ 47.5, 14.0 });
	auto const viaEpsg = epsg.d->Transform({ 47.5, 14.0 });
	EXPECT_NEAR(viaEpsg[0], viaTowgs84[0], 1e-7);
	EXPECT_NEAR(viaEpsg[1], viaTowgs84[1], 1e-7);

	// And the default (Position Vector) reading of those same numbers is meaningfully different.
	auto const viaDefault = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(mgiCoordinateFrame), Crs(wgs84))->GetMathTransform()->Transform({ 47.5, 14.0 });
	EXPECT_GT(std::hypot(viaDefault[0] - viaTowgs84[0], viaDefault[1] - viaTowgs84[1]), 1e-6);
}
