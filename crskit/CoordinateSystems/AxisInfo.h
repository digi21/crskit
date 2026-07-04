#pragma once

#include <compare>
#include <format>
#include <string>
#include <vector>

#include "../StringUtil.h"
#include "../TokenWkt.h"
#include "AxisOrientationEnum.h"

namespace CrsKit::CoordinateSystems
{
	class AxisInfo final
	{
		std::string _name;
		AxisOrientationEnum _orientation;

	public:
		AxisInfo() = default;

		AxisInfo(std::string const& name, AxisOrientationEnum orientation)
			: _name{name}
			, _orientation{orientation}
		{
		}

		auto operator<=>(AxisInfo const&) const = default;

		static auto FromWkt(Wkt::TokenWkt const& tokenPROJCS) -> std::vector<AxisInfo>
		{
			std::vector<AxisInfo> axis;

			for (auto i = 0; i < tokenPROJCS.GetChildCount(); i++)
			{
				try
				{
					if (0 == compareNoCase(tokenPROJCS.GetChild(i).GetName().c_str(), "AXIS"))
					{
						auto const orientationName = tokenPROJCS.GetChild(i).GetChild(1).GetString();
						AxisOrientationEnum orientation;

						if (0 == compareNoCase(orientationName.c_str(), "NORTH"))
							orientation = AxisOrientationEnum::North;
						else if (0 == compareNoCase(orientationName.c_str(), "SOUTH"))
							orientation = AxisOrientationEnum::South;
						else if (0 == compareNoCase(orientationName.c_str(), "EAST"))
							orientation = AxisOrientationEnum::East;
						else if (0 == compareNoCase(orientationName.c_str(), "WEST"))
							orientation = AxisOrientationEnum::West;
						else if (0 == compareNoCase(orientationName.c_str(), "UP"))
							orientation = AxisOrientationEnum::Up;
						else if (0 == compareNoCase(orientationName.c_str(), "DOWN"))
							orientation = AxisOrientationEnum::Down;
						else
							orientation = AxisOrientationEnum::Other;

						axis.push_back(AxisInfo(tokenPROJCS.GetChild(i).GetChild(0).GetString(), orientation));
					}
				}
				catch (...)
				{
				}
			}

			return axis;
		}

	public:
		auto GetName() const -> std::string { return _name; }
		auto SetName(std::string const& name) -> void { _name = name; }

		auto GetOrientation() const -> AxisOrientationEnum { return _orientation; }
		auto SetOrientation(AxisOrientationEnum orientation) -> void { _orientation = orientation; }

		auto GetWkt() const -> std::string
		{
			return std::format("AXIS[\"{}\",{}]",
						GetName().c_str(),
						ToString(GetOrientation()).c_str());
		}

		auto CompareTo(AxisInfo const& obj) const -> int
		{
			auto const value = compareNoCase(_name.c_str(), obj._name.c_str());
			if (0 != value)
				return value;

			return static_cast<int>(_orientation) - static_cast<int>(obj.GetOrientation());
		}
	};
}
