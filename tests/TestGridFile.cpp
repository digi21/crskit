// Verifies that a missing grid file (geoid/NTv2/NADCON) raises a typed GridFileNotFoundException
// carrying enough information to locate the file (name, searched path, and, when reached through an
// EPSG operation, the operation metadata + INFORMATION_SOURCE).

#include "TestBase.h"
#include "../crskit/CoordinateTransformations/Algorithms/GridFiles/Inflate.h"

#include <bit>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>

using namespace CrsKit;

namespace
{
	// Write `value` big-endian to the stream (GTX is big-endian).
	template <typename T>
	void writeBigEndian(std::ostream& os, T value)
	{
		char bytes[sizeof(T)];
		std::memcpy(bytes, &value, sizeof(T));
		if constexpr (std::endian::native == std::endian::little)
			std::reverse(std::begin(bytes), std::end(bytes));
		os.write(bytes, sizeof(T));
	}

	// Point the ambient data directory at a folder for the duration of a test, then restore it.
	struct DataDirectoryGuard
	{
		std::shared_ptr<CrsContext> context{ GetDefaultContext() };
		std::string saved{ context->dataDirectory };
		explicit DataDirectoryGuard(std::string dir) { context->dataDirectory = std::move(dir); }
		~DataDirectoryGuard() { context->dataDirectory = saved; }
	};
}
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

// Building an NTv2 transform from WKT that references a non-existent grid file must throw the typed
// exception with the grid file name and the path that was searched.
TEST(GridFile, MissingGridThrowsTypedException)
{
	try
	{
		static_cast<void>(GetMathTransformFactory()->CreateFromWkt(R"WKT(PARAM_MT["NTv2",PARAMETER["Latitude_and_longitude_difference_file","__does_not_exist__.gsb"]])WKT"));
		FAIL() << "expected GridFileNotFoundException";
	}
	catch (GridFileNotFoundException const& e)
	{
		EXPECT_EQ("__does_not_exist__.gsb", e.gridFileName);
		EXPECT_FALSE(e.searchedPath.empty());
	}
}

// The enriched form carries the EPSG operation metadata (code, name, area, information source) in its message.
TEST(GridFile, EnrichedFormCarriesOperationMetadata)
{
	GridFileNotFoundException const e{ "grid.gsb", "/data/grid.gsb", 1803, "OSGB36 to WGS 84 (NTv2)", "United Kingdom", "Ordnance Survey" };

	EXPECT_EQ(1803, e.operationCode);
	EXPECT_EQ("Ordnance Survey", e.informationSource);

	std::string const message = e.what();
	EXPECT_NE(std::string::npos, message.find("1803"));
	EXPECT_NE(std::string::npos, message.find("Ordnance Survey"));
	EXPECT_NE(std::string::npos, message.find("grid.gsb"));
}

// A compound CRS whose vertical is Alicante height (EPSG 5782, the Spanish mainland orthometric height)
// transformed to WGS 84 3D must succeed. Its geoid operation (EPSG 9410, EGM2008 / REDNAP) is catalogued
// with source ETRS89-ESP [REGENTE], not the generic ETRS89/WGS 84, so the datum-equivalent fallback must
// locate it. Before the fix this threw "No transformations between WGS 84 and Alicante height have been found".
// (Requires the REDNAP geoid grid EGM08_REDNAP.txt in the data directory.)
TEST(GridFile, CompoundAlicanteHeightToWgs84UsesRednapGeoid)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(25830, 5782);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4979);
	ASSERT_NE(nullptr, source);
	ASSERT_NE(nullptr, target);

	auto const ct = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
		std::dynamic_pointer_cast<CoordinateSystem>(source),
		std::dynamic_pointer_cast<CoordinateSystem>(target));
	ASSERT_NE(nullptr, ct);

	// ETRS89 / UTM zone 30N easting/northing in central mainland Spain, orthometric height 0.
	auto const out = ct->GetMathTransform()->Transform(std::vector<double>{ 500000.0, 4400000.0, 0.0 });
	ASSERT_EQ(3u, out.size());

	// WGS 84 latitude / longitude / ellipsoidal height. With H = 0, h equals the geoid undulation
	// N (about +50 m in mainland Spain), so the geoid grid was actually applied.
	EXPECT_GT(out[0], 35.0);  EXPECT_LT(out[0], 44.0);   // latitude
	EXPECT_GT(out[1], -10.0); EXPECT_LT(out[1], 5.0);    // longitude
	EXPECT_GT(out[2], 30.0);  EXPECT_LT(out[2], 70.0);   // ellipsoidal height ~ geoid undulation
}

// The EGM2008 global geoid grids are named in the EPSG catalogue with or without the ".gz" download
// suffix (e.g. "Und_min1x1_egm2008_isw=82_WGS84_TideFree"), and come in several arc-minute
// resolutions. The loader must recognise all of these, deriving the grid size from the resolution in
// the name. "Recognised" here means it does NOT reject the name as an unsupported format: it either
// builds (grid installed) or reports the grid missing (GridFileNotFoundException).
TEST(GridFile, Egm2008GlobalGridNamesAreRecognized)
{
	for (auto const* const name : { "Und_min1x1_egm2008_isw=82_WGS84_TideFree",
	                                "Und_min1x1_egm2008_isw=82_WGS84_TideFree.gz",
	                                "Und_min2.5x2.5_egm2008_isw=82_WGS84_TideFree" })
	{
		auto const wkt = std::format(
			R"WKT(PARAM_MT["Geographic3DToGravityRelatedHeightEGM2008",PARAMETER["geoid_model_file","{}"]])WKT", name);
		try
		{
			static_cast<void>(GetMathTransformFactory()->CreateFromWkt(wkt));
			// Built successfully: the grid is installed. Fine — the name was recognised.
		}
		catch (GridFileNotFoundException const&) { /* recognised, grid not installed: fine */ }
		catch (UnsupportedFormatException const&) { FAIL() << "EGM2008 name not recognised: " << name; }
	}
}

// A synthetic ".gtx" geoid grid must be read (big-endian header + data, stored south-to-north) and
// bilinearly interpolated, then applied by the (gtx) geoid method to convert ellipsoidal to
// orthometric height. Exercises the whole gtx path: method registration, content/name recognition,
// the binary reader (endianness + row orientation) and the interpolation.
TEST(GridFile, GtxSyntheticGridIsReadAndInterpolated)
{
	namespace fs = std::filesystem;
	auto const dir = fs::temp_directory_path() / "crskit_gtx_test";
	fs::create_directories(dir);
	auto const file = dir / "synthetic.gtx";
	{
		std::ofstream os{ file, std::ios::binary };
		writeBigEndian<double>(os, 40.0);       // south-edge latitude
		writeBigEndian<double>(os, -4.0);       // west-edge longitude
		writeBigEndian<double>(os, 1.0);        // latitude step
		writeBigEndian<double>(os, 1.0);        // longitude step
		writeBigEndian<std::int32_t>(os, 2);    // rows
		writeBigEndian<std::int32_t>(os, 2);    // columns
		writeBigEndian<float>(os, 10.0f); writeBigEndian<float>(os, 20.0f);  // south row (lat 40): W, E
		writeBigEndian<float>(os, 30.0f); writeBigEndian<float>(os, 40.0f);  // north row (lat 41): W, E
	}

	auto dirStr = dir.string();
	dirStr += '/';
	DataDirectoryGuard guard{ dirStr };

	auto const mt = GetMathTransformFactory()->CreateFromWkt(
		R"WKT(PARAM_MT["Geographic3DToGravityRelatedHeightGtx",PARAMETER["geoid_model_file","synthetic.gtx"]])WKT");
	ASSERT_NE(nullptr, mt);

	// Grid centre (lon -3.5, lat 40.5): bilinear of {10,20,30,40} = 25. So H = h - N = 100 - 25 = 75.
	auto const out = mt->Transform(std::vector<double>{ -3.5, 40.5, 100.0 });
	ASSERT_EQ(1u, out.size());
	EXPECT_NEAR(75.0, out[0], 1e-6);

	std::error_code ec;
	fs::remove(file, ec);
	fs::remove(dir, ec);
}

// The built-in raw-DEFLATE decoder (used to read compressed GeoTIFF grid tiles) must reproduce a
// stream compressed by zlib exactly. Vector: a reference string, raw-deflated (RFC 1951, no wrapper).
TEST(GridFile, InflateRawDecodesDeflateStream)
{
	static const unsigned char compressed[] = {
		0x73,0x2e,0x2a,0xf6,0xce,0x2c,0x51,0xc8,0xcc,0x4b,0xcb,0x49,0x2c,0x49,0x55,0x28,
		0x49,0x2d,0x2e,0xb1,0x52,0x28,0xc9,0x48,0x55,0x28,0x2c,0xcd,0x4c,0xce,0x56,0x48,
		0x2a,0xca,0x2f,0xcf,0x53,0x48,0xcb,0xaf,0x50,0xc8,0x2a,0xcd,0x2d,0x28,0x56,0xc8,
		0x2f,0x4b,0x2d,0x02,0x4b,0xe7,0x24,0x56,0x55,0x2a,0xa4,0xe4,0xa7,0xeb,0x29,0x38,
		0x0f,0x0b,0x13,0x00 };
	std::string const unit = "CrsKit inflate test: the quick brown fox jumps over the lazy dog. ";
	std::string const expected = unit + unit + unit + unit;

	std::vector<unsigned char> out(expected.size());
	ASSERT_TRUE(CrsKit::CoordinateTransformations::Algorithms::InflateRaw(
		compressed, sizeof(compressed), out.data(), out.size()));
	EXPECT_EQ(expected, std::string(out.begin(), out.end()));
}

// A geodetic GeoTIFF (".tif") geoid grid must be read — TIFF tiles, DEFLATE, the floating-point
// predictor and PixelIsPoint georeferencing — and produce the reference undulations at known nodes.
// Verified against the values GDAL/PROJ read from the same file. Skipped when the grid is not
// installed (it is not versioned). Input is {longitude, latitude, ellipsoidal height}; the result is
// the gravity-related height H = h - N, so at h = 0 the output equals -N.
TEST(GridFile, GtgGeoTiffGridMatchesReference)
{
	namespace fs = std::filesystem;
	auto const dir = GetDefaultContext()->dataDirectory;
	if (!fs::exists(fs::path(dir) / "mx_inegi_GGM10.tif"))
		GTEST_SKIP() << "mx_inegi_GGM10.tif not installed in the data directory";

	auto const mt = GetMathTransformFactory()->CreateFromWkt(
		R"WKT(PARAM_MT["Geographic3DToGravityRelatedHeightGtg",PARAMETER["geoid_model_file","mx_inegi_GGM10.tif"]])WKT");
	ASSERT_NE(nullptr, mt);

	auto undulationAt = [&](double lon, double lat)
	{
		auto const out = mt->Transform(std::vector<double>{ lon, lat, 0.0 });
		return -out[0];   // H = h - N with h = 0  =>  N = -H
	};

	EXPECT_NEAR(-15.54, undulationAt(-102.4792, 23.4792), 0.05);
	EXPECT_NEAR(-33.77, undulationAt(-110.6458, 24.6458), 0.05);
	EXPECT_NEAR(-12.76, undulationAt(-102.3125, 20.4792), 0.05);
}

// An AUSGeoid NTv2 (".gsb") geoid grid: its first grid field holds the geoid separation N in metres,
// stored in NTv2's south-to-north, positive-west (east-to-west) node order. Verified against exact
// node values read from the same file. Skipped when the grid is not installed (not versioned).
TEST(GridFile, AusGeoidNtv2GridMatchesReference)
{
	namespace fs = std::filesystem;
	auto const dir = GetDefaultContext()->dataDirectory;
	if (!fs::exists(fs::path(dir) / "AUSGeoid2020_20180201.gsb"))
		GTEST_SKIP() << "AUSGeoid2020_20180201.gsb not installed in the data directory";

	auto const mt = GetMathTransformFactory()->CreateFromWkt(
		R"WKT(PARAM_MT["Geographic3DToGravityRelatedHeightAUSGeoidV2",PARAMETER["geoid_model_file","AUSGeoid2020_20180201.gsb"]])WKT");
	ASSERT_NE(nullptr, mt);

	auto undulationAt = [&](double lon, double lat)
	{
		return -mt->Transform(std::vector<double>{ lon, lat, 0.0 })[0];
	};

	EXPECT_NEAR(-32.8640, undulationAt(117.983333, -35.316667), 0.001);
	EXPECT_NEAR( 10.4110, undulationAt(133.150000, -24.150000), 0.001);
	EXPECT_NEAR( 29.8110, undulationAt(151.150000, -31.983333), 0.001);
}

// A geoid_model_file that is neither a REDNAP nor an EGM2008 grid name is rejected as unsupported.
TEST(GridFile, UnknownGeoidGridNameIsUnsupported)
{
	try
	{
		static_cast<void>(GetMathTransformFactory()->CreateFromWkt(
			R"WKT(PARAM_MT["Geographic3DToGravityRelatedHeightEGM2008",PARAMETER["geoid_model_file","totally_bogus_grid.dat"]])WKT"));
		FAIL() << "expected UnsupportedFormatException";
	}
	catch (UnsupportedFormatException const&) { /* as expected */ }
}

// Candidate operations now carry their EPSG area of use and the grid file(s) they require, so a caller
// (e.g. a transformation picker) can show both. The "WGS 84 to EGM2008 height" operations (EGM2008
// height = EPSG 3855) must report a non-empty area and the EGM2008 geoid grid as their required file.
TEST(GridFile, CandidateOperationsCarryAreaOfUseAndGridFile)
{
	auto const provider = GetCurrentEnvironment()->Provider;
	ASSERT_NE(provider, nullptr);

	auto const operations = provider->GeographicToVerticalOperationCodes(3855);
	ASSERT_FALSE(operations.empty());

	bool anyArea{ false };
	bool foundEgm2008Grid{ false };
	for (auto const& op : operations)
	{
		if (!op.AreaOfUse.empty())
			anyArea = true;
		for (auto const& file : op.GridFiles)
			if (file.find("egm2008") != std::string::npos)
				foundEgm2008Grid = true;
	}

	EXPECT_TRUE(anyArea) << "no candidate reported an area of use";
	EXPECT_TRUE(foundEgm2008Grid) << "no candidate reported an EGM2008 grid file";
}

// The EPSG dataset version is read straight from the database's version history (e.g. "12.057").
TEST(EpsgCatalogue, GetEpsgVersionReturnsAVersion)
{
	auto const version = GetEpsgVersion();
	EXPECT_FALSE(version.empty()) << "no EPSG dataset version reported";
	EXPECT_NE(version.find('.'), std::string::npos) << "unexpected version format: " << version;
}
