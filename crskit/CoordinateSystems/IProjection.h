#pragma once

#include <string>
#include <vector>

#include "../Parameter.h"

namespace CrsKit::CoordinateSystems
{
	///	<summary>
	///		Describes a projection from geographic coordinates to projected coordinates.
	///	</summary>
	///	<remarks>
	///		Projected projection transformations are used by projected coordinate systems to convert coordinates
	///		geographic (longitude and latitude) into (X,Y) coordinates. These (X,Y) coordinates can be imagined as lying on a plane, like a map
	///		on paper or on a screen.
	///
	///		All projected projection transformations will have the following properties:
	///		All projected projection transformations will have the following properties:
	///
	///		1. Convierten coordinates (Longitude, Latitude) a (X,Y).
	///		2. All angles are assumed to be sexagesimal (degree and decimal fraction of a degree), and all distances are assumed to be metres.
	///		3. The domain should be a subset of {[-180, 180)x(-90,90), (0,-90),(0x90)}.
	///
	///		Although all projected projection transformations must have the properties mentioned above, many coordinate systems
	///		projected ones have different properties. For example, in Europe some projected coordinate systems use centesimal degrees instead of 
	///		sexagesimal, and often the base geographic coordinate system is (Latitude, Longitude) instead of (Longitude, Latitude). This indicates that the transformation
	///		projected projection is used as one step of a series of transformations, where the other steps change units and swap coordinates.
	///	</remarks>
	struct IProjection
	{
		///	<summary>
		///		Returns the classification of the projection (e.g. "Transverse_Mercator");
		///	</summary>
		[[nodiscard]] virtual auto GetClassName() const -> std::string = 0;

		///	<summary>
		///		Returns the number of parameters of the projection.
		///	</summary>
		[[nodiscard]] virtual auto GetNumParameters() const -> int = 0;

		///	<summary>
		///		Gets a projection parameter by its index.
		///	</summary>
		[[nodiscard]] virtual auto GetParameter(int index) const -> Parameter = 0;

		[[nodiscard]] virtual auto GetParameter(std::string const& name) const -> Parameter = 0;
		[[nodiscard]] virtual auto GetParameters() const noexcept -> std::vector<Parameter> const& = 0;
	};
}
