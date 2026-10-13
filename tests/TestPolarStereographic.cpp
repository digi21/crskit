// Polar Stereographic variants A, B and C. A and B against PROJ 9.8.1; C, which PROJ does not implement, against the GN7-2
// example and against PROJ variant B with the false northing moved by rhoF (C = B with FN = NF - rhoF at the south pole).
// Not part of the GIGS suite (no "Test5xxx" prefix).

#include <regex>
#include <string>
#include <vector>

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	struct PolarCase
	{
		int crs;
		int base;
		double latitude;
		double longitude;
		double first;
		double second;
		double tolerance;
	};

	class PolarStereographic : public ::testing::TestWithParam<PolarCase> {};
}

TEST_P(PolarStereographic, ForwardAndReverse)
{
	auto const& c = GetParam();
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const forward = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		factory->CreateCoordinateSystem(c.base), factory->CreateCoordinateSystem(c.crs))->GetMathTransform();

	auto const projected = forward->Transform({ c.latitude, c.longitude });
	EXPECT_NEAR(c.first, projected[0], c.tolerance);
	EXPECT_NEAR(c.second, projected[1], c.tolerance);

	auto const geographic = forward->GetInverse()->Transform({ projected[0], projected[1] });
	EXPECT_NEAR(c.latitude, geographic[0], 1e-9);
	EXPECT_NEAR(c.longitude, geographic[1], 1e-9);
}

namespace
{
	struct ScopedWkt2
	{
		WktVersion previous{ GetDefaultContext()->wktVersion };
		ScopedWkt2() { GetDefaultContext()->wktVersion = WktVersion::Wkt2_2019; }
		~ScopedWkt2() { GetDefaultContext()->wktVersion = previous; }
	};

	// Projects (lat, lon) with a projected CRS parsed from its own WKT without identifiers, so the parser builds it from
	// its parts instead of taking the catalogue entry.
	auto ProjectThroughWkt(std::string wkt, double latitude, double longitude) -> std::vector<double>
	{
		wkt = std::regex_replace(wkt, std::regex{ R"(,(AUTHORITY|ID)\["EPSG",("?)\d+("?)\])" }, "");
		auto const parsed = std::dynamic_pointer_cast<CoordinateSystems::ProjectedCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt));
		EXPECT_NE(nullptr, parsed) << wkt;
		if (!parsed)
			return {};
		auto const geographic = std::dynamic_pointer_cast<CoordinateSystems::CoordinateSystem>(parsed->GetGeographicCoordinateSystem());
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geographic, parsed)->GetMathTransform()->Transform({ latitude, longitude });
	}
}

// The WKT1 and WKT2 of each variant parse back to the same projection.
TEST_P(PolarStereographic, WktRoundTrip)
{
	auto const& c = GetParam();
	auto const crs = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(c.crs);
	for (auto const wkt2 : { false, true })
	{
		std::string wkt;
		if (wkt2)
		{
			ScopedWkt2 const scoped;
			wkt = crs->GetWkt();
		}
		else
			wkt = crs->GetWkt();

		auto const projected = ProjectThroughWkt(wkt, c.latitude, c.longitude);
		ASSERT_EQ(2u, projected.size()) << wkt;
		EXPECT_NEAR(c.first, projected[0], c.tolerance) << wkt;
		EXPECT_NEAR(c.second, projected[1], c.tolerance) << wkt;
	}
}
INSTANTIATE_TEST_SUITE_P(Variants, PolarStereographic, ::testing::Values(
	PolarCase{ 2986, 4637, -(66 + 36 / 60.0 + 18.820 / 3600), 140 + 4 / 60.0 + 17.040 / 3600, 303169.52, 244055.72, 0.01 }, // C, GN7-2 example
	PolarCase{ 2985, 4636, -66.44, 140.47, 321014.9793047901, 262429.3064700635, 1e-3 },   // C, Petrels 1972 / Terre Adelie
	PolarCase{ 7082, 7073, -66.37, 139.0, 255154.81611351037, 269943.85632705037, 1e-3 }, // C, RGTAAF07 / Terre Adelie
	PolarCase{ 32661, 4326, 73.0, 44.0, 3320416.747359853, 632668.4312721284, 1e-3 },     // A, WGS 84 / UPS North (GN7-2 example)
	PolarCase{ 3031, 4326, -75.0, 120.0, 1419227.9157567972, -819391.6192036179, 1e-3 },  // B, WGS 84 / Antarctic Polar Stereographic
	PolarCase{ 3413, 4326, 70.0, -30.0, 566277.3449397993, -2113375.8224905375, 1e-3 })); // B, WGS 84 / NSIDC Sea Ice Polar Stereographic North