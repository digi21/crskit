// WKT1 of a geocentric CRS (GEOCCS): three axes out, three axes in. Not part of the GIGS suite (no "Test5xxx"
// prefix).

#include <string>

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	std::string const kGeoccsHead =
		R"WKT(GEOCCS["WGS 84",DATUM["World Geodetic System 1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["metre",1])WKT";
}

TEST(GeocentricWkt, RoundTripKeepsTheThreeAxes)
{
	for (auto const code : { 4978, 4334, 4384, 4884, 4910 })
	{
		auto const original = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code);
		ASSERT_EQ(3, original->GetDimension()) << code;

		auto const parsed = GetCoordinateSystemFactory()->CreateFromWkt(original->GetWkt());
		ASSERT_EQ(3, parsed->GetDimension()) << code;
		for (auto i = 0; i < 3; i++)
		{
			EXPECT_EQ(original->GetAxis(i).GetName(), parsed->GetAxis(i).GetName()) << code << " axis " << i;
			EXPECT_EQ(original->GetAxis(i).GetOrientation(), parsed->GetAxis(i).GetOrientation()) << code << " axis " << i;
		}
	}
}

// Without AXIS, WKT1 (OGC 01-009) defines X OTHER, Y EAST, Z NORTH.
TEST(GeocentricWkt, NoAxisGivesTheDefaultAxes)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kGeoccsHead + "]");
	ASSERT_EQ(3, cs->GetDimension());
	EXPECT_EQ(CoordinateSystems::AxisOrientationEnum::Other, cs->GetAxis(0).GetOrientation());
	EXPECT_EQ(CoordinateSystems::AxisOrientationEnum::East, cs->GetAxis(1).GetOrientation());
	EXPECT_EQ(CoordinateSystems::AxisOrientationEnum::North, cs->GetAxis(2).GetOrientation());
}

TEST(GeocentricWkt, TwoAxesAreRejected)
{
	EXPECT_THROW((void)GetCoordinateSystemFactory()->CreateFromWkt(kGeoccsHead + R"(,AXIS["X",OTHER],AXIS["Y",EAST]])"), WktParseException);
}
