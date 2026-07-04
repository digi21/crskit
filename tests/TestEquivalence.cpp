// Structural equivalence of coordinate systems (AreEquivalent) and the factory shortcut it powers:
// same definition with a different name/authority is equivalent (WKT comparison missed this), and
// systems differing only in axis order are equivalent -> the library produces the axis swap itself.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	auto Crs(char const* wkt) { return GetCoordinateSystemFactory()->CreateFromWkt(wkt); }

	char const* kWgs84 = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	// Same mathematical definition, different CRS name (and no authority).
	char const* kWgs84OtherName = R"WKT(GEOGCS["My WGS84 copy",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	// Same definition, axes swapped (Long, Lat).
	char const* kWgs84SwappedAxes = R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Long", EAST],AXIS["Lat", NORTH]])WKT";
	// Different datum/ellipsoid.
	char const* kEd50 = R"WKT(GEOGCS["ED50",DATUM["European_Datum_1950",SPHEROID["International 1924",6378388,297]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";

	// Two authority-less systems with DIFFERENT datums but the SAME axes. Names made up so they do not
	// resolve to a catalogue entry; only the datum can tell them apart.
	char const* kCustomDatumA = R"WKT(GEOGCS["My CRS A",DATUM["My_Datum_A",SPHEROID["International 1924",6378388,297]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
	char const* kCustomDatumB = R"WKT(GEOGCS["My CRS B",DATUM["My_Datum_B",SPHEROID["Clarke 1866",6378206.4,294.9786982138982]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat", NORTH],AXIS["Long", EAST]])WKT";
}

TEST(Equivalence, SameDefinitionDifferentNameIsEquivalent)
{
	EXPECT_TRUE(AreEquivalent(Crs(kWgs84), Crs(kWgs84OtherName)));
}

TEST(Equivalence, SwappedAxesIsEquivalent)
{
	EXPECT_TRUE(AreEquivalent(Crs(kWgs84), Crs(kWgs84SwappedAxes)));
}

TEST(Equivalence, DifferentDatumIsNotEquivalent)
{
	EXPECT_FALSE(AreEquivalent(Crs(kWgs84), Crs(kEd50)));
}

// GeographicCoordinateSystem::CompareTo must look at the datum, not only authority/axes: two
// authority-less systems with the same axes but different datums are NOT the same CRS. Otherwise the
// transform factory's same-datum (un)projection shortcut would fire and silently skip the datum shift.
TEST(Equivalence, CompareToDistinguishesAuthoritylessDatums)
{
	auto const a = std::dynamic_pointer_cast<GeographicCoordinateSystem>(Crs(kCustomDatumA));
	auto const b = std::dynamic_pointer_cast<GeographicCoordinateSystem>(Crs(kCustomDatumB));
	ASSERT_NE(nullptr, a);
	ASSERT_NE(nullptr, b);

	EXPECT_NE(0, a->CompareTo(b));
	EXPECT_NE(0, b->CompareTo(a));

	// And an equivalent copy of the same definition still compares equal.
	EXPECT_EQ(0, a->CompareTo(std::dynamic_pointer_cast<GeographicCoordinateSystem>(Crs(kCustomDatumA))));
}

// CompareCrs grades partial matches instead of a yes/no: a different datum but the same prime meridian
// and angular unit scores 40 (= 20 + 20, the datum's weight of 60 withheld), and a full match scores 100.
// AreEquivalent is exactly the CompareCrs == 100 boundary.
TEST(Equivalence, CompareCrsGradesPartialMatches)
{
	EXPECT_EQ(100, CompareCrs(Crs(kWgs84), Crs(kWgs84OtherName)));

	auto const partial = CompareCrs(Crs(kWgs84), Crs(kEd50));
	EXPECT_GT(partial, 0);
	EXPECT_LT(partial, 100);
	EXPECT_EQ(40, partial);

	EXPECT_TRUE(AreEquivalent(Crs(kWgs84), Crs(kWgs84OtherName)));
	EXPECT_FALSE(AreEquivalent(Crs(kWgs84), Crs(kEd50)));
}

// End-to-end: the library builds the transform between the same CRS with swapped axes (an axis swap),
// without looking up any EPSG operation and without throwing.
TEST(Equivalence, FactoryResolvesSwappedAxesItself)
{
	auto const ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Crs(kWgs84), Crs(kWgs84SwappedAxes));
	ASSERT_NE(nullptr, ct);
	auto const mt = ct->GetMathTransform();
	ASSERT_NE(nullptr, mt);

	// (Lat=40, Long=-3) in source order -> (Long=-3, Lat=40) in target order.
	auto const r = mt->Transform({ 40.0, -3.0 });
	EXPECT_NEAR(-3.0, r[0], 1e-9);
	EXPECT_NEAR(40.0, r[1], 1e-9);
}
