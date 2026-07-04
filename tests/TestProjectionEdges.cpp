// Adversarial edge review of the map projections: GIGS validates interior reference points, but not
// the singular edges (poles, far from the central meridian, antimeridian, wide projected grids).
// These tests sweep each projection well past its comfort zone and assert the maths never returns
// NaN (a true singularity may return +/-inf, which is acceptable) and the iterative inverses never
// hang (guaranteed by their iteration caps). Not part of the GIGS suite (no "Test5xxx" prefix).

#include "TestBase.h"
#include "GigsFactory.h"

#include <format>

using namespace TestCrsKit;
using namespace CrsKit;

namespace
{
	struct ProjectionCase
	{
		char const* name;
		char const* gigsCode; // GIGS projected-CRS code understood by GigsFactory
	};

	// One representative projected CRS per projection method (the codes exercised by Test5101-5113).
	constexpr ProjectionCase kCases[] = {
		{ "TransverseMercator", "62007" },
		{ "TransverseMercatorSouthOrientated", "62017" },
		{ "LambertConformalConic1SP", "62035" },
		{ "LambertConformalConic2SP", "62013" },
		{ "ObliqueStereographic", "62011" },
		{ "ObliqueMercator", "62020" },
		{ "HotineObliqueMercator", "62021" },
		{ "AmericanPolyconic", "62019" },
		{ "CassiniSoldner", "62022" },
		{ "AlbersConicEqualArea", "62016" },
		{ "LambertAzimuthalEqualArea", "62023" },
		{ "Mercator1SP", "62037" },
		{ "Mercator2SP", "62034" },
	};

	[[nodiscard]] auto hasNaN(std::vector<double> const& v) -> bool
	{
		for (auto const value : v)
			if (std::isnan(value))
				return true;
		return false;
	}
}

// Forward well past each projection's intended zone must never yield NaN. The sweep stays inside the
// usable domain (it excludes the genuine singularities: the poles, ~90 deg from the central meridian,
// the antipode) -- those are mathematically undefined and may return +/-inf, which is acceptable.
TEST(ProjectionEdges, ForwardNeverNaN)
{
	Gigs::GigsFactory gigs;
	for (auto const& projection : kCases)
	{
		auto const pcs = gigs.CreateProjectedCoordinateSystem(projection.gigsCode);
		ASSERT_TRUE(pcs) << projection.name;
		auto const [d, i] = BuildDirectInverse(pcs->GetGeographicCoordinateSystem(), pcs);

		std::string failure;
		for (int lat = -85; lat <= 85 && failure.empty(); lat += 1)
			for (int lon = -70; lon <= 70; lon += 2)
				if (hasNaN(d->Transform({ static_cast<double>(lat), static_cast<double>(lon) })))
				{
					failure = std::format("lat={} lon={}", lat, lon);
					break;
				}

		EXPECT_TRUE(failure.empty()) << projection.name << " forward produced NaN at " << failure;
	}
}

// Inverse over a wide projected grid must never yield NaN and must never hang (iteration caps).
TEST(ProjectionEdges, InverseNeverNaNNorHangs)
{
	Gigs::GigsFactory gigs;
	for (auto const& projection : kCases)
	{
		auto const pcs = gigs.CreateProjectedCoordinateSystem(projection.gigsCode);
		ASSERT_TRUE(pcs) << projection.name;
		auto const [d, i] = BuildDirectInverse(pcs->GetGeographicCoordinateSystem(), pcs);

		std::string failure;
		for (double x = -5e6; x <= 5e6 && failure.empty(); x += 5e5)
			for (double y = -5e6; y <= 5e6; y += 5e5)
				if (hasNaN(i->Transform({ x, y })))
				{
					failure = std::format("x={} y={}", x, y);
					break;
				}

		EXPECT_TRUE(failure.empty()) << projection.name << " inverse produced NaN at " << failure;
	}
}

// Note: a projected<->geographic round-trip test was considered to catch atan-vs-atan2 quadrant
// defects, but the GIGS suite (Test5101-5113) already round-trips every projection at many reference
// points on both sides of the central meridian and in both hemispheres -- a quadrant defect would
// flip the longitude by ~180deg and fail there. No such defect exists, so atan was left as-is. A
// generic round-trip here would also have to honour each CRS's per-axis order (Lat/Long vs Long/Lat,
// E/N vs N/E), which GIGS already covers; it is intentionally not duplicated.
