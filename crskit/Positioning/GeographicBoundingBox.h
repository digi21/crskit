#pragma once

#include <cmath>

namespace CrsKit::Positioning
{
	// Rectangle of an extent of the European Petroleum Survey Group (EPSG) catalogue (columns BBOX_*_BOUND_* of
	// epsg_extent), in degrees. Latitudes range from -90 to 90 degrees and longitudes from -180 to 180 degrees, both ends
	// included. When WestBoundLongitudeDegrees is greater than EastBoundLongitudeDegrees, the rectangle crosses the
	// antimeridian and covers the longitudes from WestBoundLongitudeDegrees to 180 and from -180 to EastBoundLongitudeDegrees.
	struct GeographicBoundingBox final
	{
		double SouthBoundLatitudeDegrees{};
		double NorthBoundLatitudeDegrees{};
		double WestBoundLongitudeDegrees{};
		double EastBoundLongitudeDegrees{};

		[[nodiscard]] auto CrossesAntimeridian() const -> bool
		{
			return WestBoundLongitudeDegrees > EastBoundLongitudeDegrees;
		}

		// True when the point is inside the rectangle or on its boundary. The longitude is reduced to the range
		// [-180, 180] before the comparison, so -190 and 170 are the same longitude. Returns false if latitudeDegrees or
		// longitudeDegrees is NaN (not a number), or if longitudeDegrees is infinite.
		[[nodiscard]] auto Contains(double latitudeDegrees, double longitudeDegrees) const -> bool
		{
			if (!(latitudeDegrees >= SouthBoundLatitudeDegrees && latitudeDegrees <= NorthBoundLatitudeDegrees) || !std::isfinite(longitudeDegrees))
				return false;

			auto const reducedLongitudeDegrees = std::remainder(longitudeDegrees, 360.0);
			// Longitudes -180 and 180 are the same meridian.
			return ContainsLongitude(reducedLongitudeDegrees) || (std::fabs(reducedLongitudeDegrees) == 180.0 && ContainsLongitude(-reducedLongitudeDegrees));
		}

		auto operator==(GeographicBoundingBox const&) const -> bool = default;

	private:
		[[nodiscard]] auto ContainsLongitude(double longitudeDegrees) const -> bool
		{
			if (CrossesAntimeridian())
				return longitudeDegrees >= WestBoundLongitudeDegrees || longitudeDegrees <= EastBoundLongitudeDegrees;
			return longitudeDegrees >= WestBoundLongitudeDegrees && longitudeDegrees <= EastBoundLongitudeDegrees;
		}
	};
}
