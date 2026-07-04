#pragma once

#include <string>

namespace CrsKit::CoordinateSystems
{
	enum class AxisOrientationEnum
	{
		Other = 0,
		North = 1,
		South = 2,
		East = 3,
		West = 4,
		Up = 5,
		Down = 6
	};

	static auto ToString(AxisOrientationEnum value) -> std::string
	{
		switch (value)
		{
		case AxisOrientationEnum::Other: return "Other";
		case AxisOrientationEnum::North: return "North";
		case AxisOrientationEnum::South: return "South";
		case AxisOrientationEnum::East: return "East";
		case AxisOrientationEnum::West: return "West";
		case AxisOrientationEnum::Up: return "Up";
		case AxisOrientationEnum::Down:
		default:
			return "Down";
		}
	}
}
