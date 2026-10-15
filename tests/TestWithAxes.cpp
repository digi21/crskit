// ModifyWithAxisEastNorth / ModifyWithAxisNorthEast reorder the horizontal axes and keep the others.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	auto ExpectAxes(std::shared_ptr<CoordinateSystem> const& cs, AxisOrientationEnum first, AxisOrientationEnum second, AxisInfo const& third) -> void
	{
		ASSERT_EQ(3, cs->GetDimension());
		EXPECT_EQ(first, cs->GetAxis(0).GetOrientation());
		EXPECT_EQ(second, cs->GetAxis(1).GetOrientation());
		EXPECT_EQ(third.GetName(), cs->GetAxis(2).GetName());
		EXPECT_EQ(third.GetOrientation(), cs->GetAxis(2).GetOrientation());
	}
}

TEST(WithAxes, Geographic3DKeepsItsHeightAxis)
{
	auto const official = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4979);
	auto const height = official->GetAxis(2);
	ASSERT_EQ(AxisOrientationEnum::Up, height.GetOrientation());

	auto const eastNorth = GetCoordinateSystemFactory()->ModifyWithAxisEastNorth(official);
	ExpectAxes(eastNorth, AxisOrientationEnum::East, AxisOrientationEnum::North, height);

	auto const northEast = GetCoordinateSystemFactory()->ModifyWithAxisNorthEast(eastNorth);
	ExpectAxes(northEast, AxisOrientationEnum::North, AxisOrientationEnum::East, height);
}
