// File paths received as std::string are UTF-8 on every platform. These tests run the file-opening code
// of the library in folders and files whose names contain characters outside ASCII (the non-ASCII
// names are written as u8"..." literals with universal character names, so they do not depend on the
// encoding of this source file).

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	namespace fs = std::filesystem;

	// "Logro\u00f1o" and "Aerotriangulaci\u00f3n".
	constexpr auto kLogronoFolderName = u8"Logro\u00f1o";
	constexpr auto kAerotriangulacionFolderName = u8"Aerotriangulaci\u00f3n";

	auto CopyUtf8BytesToString(std::u8string_view text) -> std::string
	{
		return std::string{ reinterpret_cast<char const*>(text.data()), text.size() };
	}

	// Creates <temporary directory>/crskit_non_ascii/<folderName>/pas 1-2 and removes crskit_non_ascii/<folderName>
	// on destruction.
	struct NonAsciiFolder
	{
		fs::path folderPath;
		fs::path workingDirectoryPath;

		explicit NonAsciiFolder(std::u8string_view folderName)
			: folderPath{ fs::temp_directory_path() / "crskit_non_ascii" / std::u8string{ folderName } }
			, workingDirectoryPath{ folderPath / "pas 1-2" }
		{
			fs::remove_all(folderPath);
			fs::create_directories(workingDirectoryPath);
		}

		~NonAsciiFolder()
		{
			std::error_code removalError;
			fs::remove_all(folderPath, removalError);
		}

		NonAsciiFolder(NonAsciiFolder const&) = delete;
		NonAsciiFolder& operator=(NonAsciiFolder const&) = delete;

		// UTF-8 path of workingDirectoryPath / fileName.
		[[nodiscard]] auto GetUtf8FilePath(std::string const& fileName) const -> std::string
		{
			return ConvertPathToUtf8String(workingDirectoryPath / fileName);
		}

		// UTF-8 path of workingDirectoryPath followed by a path separator (the form CrsContext::dataDirectory requires).
		[[nodiscard]] auto GetUtf8WorkingDirectoryPathWithSeparator() const -> std::string
		{
			return ConvertPathToUtf8String(workingDirectoryPath / "");
		}
	};

	// Sets context->dataDirectory in the constructor and restores the previous directory path in the destructor.
	struct DataDirectoryGuard
	{
		std::shared_ptr<CrsContext> context{ GetDefaultContext() };
		std::string previousDataDirectory{ context->dataDirectory };
		explicit DataDirectoryGuard(std::string directory) { context->dataDirectory = std::move(directory); }
		~DataDirectoryGuard() { context->dataDirectory = previousDataDirectory; }
	};

	template <typename T>
	void WriteBigEndian(std::ostream& os, T value)
	{
		char bytes[sizeof(T)];
		std::memcpy(bytes, &value, sizeof(T));
		if constexpr (std::endian::native == std::endian::little)
			std::reverse(std::begin(bytes), std::end(bytes));
		os.write(bytes, sizeof(T));
	}

	template <typename T>
	void WriteLittleEndian(std::ostream& os, T value)
	{
		char bytes[sizeof(T)];
		std::memcpy(bytes, &value, sizeof(T));
		if constexpr (std::endian::native == std::endian::big)
			std::reverse(std::begin(bytes), std::end(bytes));
		os.write(bytes, sizeof(T));
	}

	// GTX grid with two rows and two columns. Bilinear interpolation at the grid centre gives a geoid undulation of 25 m.
	void WriteSyntheticGtx(fs::path const& filePath)
	{
		std::ofstream os{ filePath, std::ios::binary };
		WriteBigEndian<double>(os, 40.0);       // south-edge latitude
		WriteBigEndian<double>(os, -4.0);       // west-edge longitude
		WriteBigEndian<double>(os, 1.0);        // latitude step
		WriteBigEndian<double>(os, 1.0);        // longitude step
		WriteBigEndian<std::int32_t>(os, 2);    // rows
		WriteBigEndian<std::int32_t>(os, 2);    // columns
		WriteBigEndian<float>(os, 10.0f); WriteBigEndian<float>(os, 20.0f);
		WriteBigEndian<float>(os, 30.0f); WriteBigEndian<float>(os, 40.0f);
	}

	// Synthetic Earth Gravitational Model 2008 (EGM2008) file containing 101 records of 200 undulation values, each set
	// to 50 m, with two 4-byte markers enclosing each record.
	void WriteSyntheticEgm2008(fs::path const& filePath)
	{
		std::ofstream os{ filePath, std::ios::binary };
		for (int row = 0; row < 101; ++row)
		{
			WriteLittleEndian<float>(os, 0.0f);
			for (int column = 0; column < 200; ++column)
				WriteLittleEndian<float>(os, 50.0f);
			WriteLittleEndian<float>(os, 0.0f);
		}
	}

	constexpr char kWkt[]{ R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]])WKT" };
}

class NonAsciiPaths : public ::testing::TestWithParam<std::u8string_view> {};

INSTANTIATE_TEST_SUITE_P(Folders, NonAsciiPaths,
	::testing::Values(std::u8string_view{ kLogronoFolderName }, std::u8string_view{ kAerotriangulacionFolderName }),
	[](::testing::TestParamInfo<std::u8string_view> const& info)
	{
		return info.index == 0 ? std::string{ "Logrono" } : std::string{ "Aerotriangulacion" };
	});

TEST(NonAsciiPathHelpers, PathHelpersConvertBetweenUtf8AndPath)
{
	// "Logro\u00f1o" in UTF-8: the byte pair C3 B1 encodes U+00F1 (n with tilde).
	std::string const utf8Text{ "Logro\xC3\xB1o" };

	EXPECT_EQ(fs::path{ kLogronoFolderName }, CreatePathFromUtf8(utf8Text));
	EXPECT_EQ(utf8Text, ConvertPathToUtf8String(fs::path{ kLogronoFolderName }));
	EXPECT_EQ(utf8Text, ConvertPathToUtf8String(CreatePathFromUtf8(utf8Text)));

	EXPECT_EQ(fs::path{ "plain/ascii.prj" }, CreatePathFromUtf8("plain/ascii.prj"));
	EXPECT_EQ("plain.prj", ConvertPathToUtf8String(fs::path{ "plain.prj" }));
}

TEST_P(NonAsciiPaths, PrjFileIsWrittenAndReadBack)
{
	NonAsciiFolder const folder{ GetParam() };
	auto const utilities = GetUtilities();

	// The extension of the file passed in is replaced by ".prj".
	auto const modelPath = folder.GetUtf8FilePath("AJUSTE PASADA 1 Y 2.dxf");

	EXPECT_THROW(static_cast<void>(utilities->LoadPrjFileForFileOrThrow(modelPath)), std::runtime_error);

	utilities->CreatePrjFileForFileOrThrow(modelPath, kWkt);
	EXPECT_TRUE(fs::exists(folder.workingDirectoryPath / "AJUSTE PASADA 1 Y 2.prj"));

	EXPECT_EQ(kWkt, utilities->LoadPrjFileForFileOrThrow(modelPath));
	EXPECT_EQ(kWkt, utilities->LoadPrjFileForFileOrLocalSystem(modelPath));
}

TEST_P(NonAsciiPaths, PrjFileErrorMessageCarriesTheUtf8Path)
{
	NonAsciiFolder const folder{ GetParam() };
	auto const utilities = GetUtilities();

	auto const modelPath = folder.GetUtf8FilePath("missing.dxf");
	auto const expectedPrjPath = ConvertPathToUtf8String(fs::path{ folder.workingDirectoryPath / "missing.prj" });

	try
	{
		static_cast<void>(utilities->LoadPrjFileForFileOrThrow(modelPath));
		FAIL() << "expected std::runtime_error";
	}
	catch (std::runtime_error const& error)
	{
		EXPECT_NE(std::string::npos, std::string{ error.what() }.find(expectedPrjPath)) << error.what();
	}

	// If the parent directory does not exist, CreatePrjFileForFileOrThrow throws an exception whose message contains the .prj file path in UTF-8.
	auto const modelFilePathInMissingDirectory = ConvertPathToUtf8String(folder.workingDirectoryPath / "does not exist" / "model.dxf");
	try
	{
		utilities->CreatePrjFileForFileOrThrow(modelFilePathInMissingDirectory, kWkt);
		FAIL() << "expected std::runtime_error";
	}
	catch (std::runtime_error const& error)
	{
		EXPECT_NE(std::string::npos, std::string{ error.what() }.find(ConvertPathToUtf8String(folder.workingDirectoryPath / "does not exist" / "model.prj"))) << error.what();
	}

	// When the .prj file does not exist, LoadPrjFileForFileOrLocalSystem returns definitions of local or unknown horizontal and vertical coordinate systems.
	EXPECT_NE(kWkt, utilities->LoadPrjFileForFileOrLocalSystem(modelPath));
}

TEST_P(NonAsciiPaths, GtxGridIsReadFromANonAsciiDataDirectory)
{
	NonAsciiFolder const folder{ GetParam() };
	auto const gridFileName = CopyUtf8BytesToString(u8"geoide_Logro\u00f1o.gtx");
	WriteSyntheticGtx(folder.workingDirectoryPath / CreatePathFromUtf8(gridFileName));

	DataDirectoryGuard const guard{ folder.GetUtf8WorkingDirectoryPathWithSeparator() };

	EXPECT_TRUE(IsGridFileInstalled(gridFileName));
	EXPECT_FALSE(IsGridFileInstalled("other.gtx"));

	auto const wkt = std::format(
		R"WKT(PARAM_MT["Geographic3DToGravityRelatedHeightGtx",PARAMETER["geoid_model_file","{}"]])WKT", gridFileName);
	auto const mathTransform = GetMathTransformFactory()->CreateFromWkt(wkt);
	ASSERT_NE(nullptr, mathTransform);

	// At longitude -3.5 degrees and latitude 40.5 degrees, geoid undulation is 25 m; an ellipsoidal height of 100 m gives a gravity-related height of 75 m.
	auto const transformedCoordinates = mathTransform->Transform(std::vector<double>{ -3.5, 40.5, 100.0 });
	ASSERT_EQ(1u, transformedCoordinates.size());
	EXPECT_NEAR(75.0, transformedCoordinates[0], 1e-6);

	// The transformation's WKT contains gridFileName encoded in UTF-8.
	EXPECT_NE(std::string::npos, mathTransform->GetWkt().find(gridFileName)) << mathTransform->GetWkt();
	EXPECT_NE(std::string::npos, mathTransform->GetInverse()->GetWkt().find(gridFileName));
}

TEST_P(NonAsciiPaths, Egm2008GridIsReadFromANonAsciiDataDirectory)
{
	NonAsciiFolder const folder{ GetParam() };
	std::string const gridFileName{ "Und_min1x1_egm2008_isw=82_WGS84_TideFree" };
	WriteSyntheticEgm2008(folder.workingDirectoryPath / gridFileName);

	DataDirectoryGuard const guard{ folder.GetUtf8WorkingDirectoryPathWithSeparator() };

	auto const wkt = std::format(
		R"WKT(PARAM_MT["Geographic3DToGravityRelatedHeightEGM2008",PARAMETER["geoid_model_file","{}"]])WKT", gridFileName);
	auto const mathTransform = GetMathTransformFactory()->CreateFromWkt(wkt);
	ASSERT_NE(nullptr, mathTransform);

	// At longitude 0 degrees and latitude 0 degrees, geoid undulation is 50 m; an ellipsoidal height of 100 m gives a gravity-related height of 50 m.
	auto const transformedCoordinates = mathTransform->Transform(std::vector<double>{ 0.0, 0.0, 100.0 });
	ASSERT_EQ(1u, transformedCoordinates.size());
	EXPECT_NEAR(50.0, transformedCoordinates[0], 1e-3);
}

TEST_P(NonAsciiPaths, MissingGridReportsTheSearchedPathInUtf8)
{
	NonAsciiFolder const folder{ GetParam() };
	DataDirectoryGuard const guard{ folder.GetUtf8WorkingDirectoryPathWithSeparator() };

	try
	{
		static_cast<void>(GetMathTransformFactory()->CreateFromWkt(
			R"WKT(PARAM_MT["NTv2",PARAMETER["Latitude_and_longitude_difference_file","__does_not_exist__.gsb"]])WKT"));
		FAIL() << "expected GridFileNotFoundException";
	}
	catch (GridFileNotFoundException const& error)
	{
		EXPECT_EQ(folder.GetUtf8WorkingDirectoryPathWithSeparator() + "__does_not_exist__.gsb", error.searchedPath);
	}
}
