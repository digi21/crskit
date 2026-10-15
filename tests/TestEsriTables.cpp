// The ESRI tables (esri_epsg.wkt, pe_list_vertcs.csv) are loaded on first use. A load that fails because
// the data directory is not set yet is retried on the next call. Each case runs in a child process, so
// the tables start unloaded there whatever the other tests have done.

#include "TestBase.h"
#include "../crskit/EsriProjectionEngine.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

using namespace CrsKit;

namespace
{
	auto TablesInstalled() -> bool
	{
		auto const directory = std::filesystem::path{ GetDefaultContext()->dataDirectory };
		return std::filesystem::exists(directory / "esri_epsg.wkt") && std::filesystem::exists(directory / "pe_list_vertcs.csv");
	}

	// Looks both tables up with a missing data directory, then with the real one from several threads at once.
	auto LookUpAfterAFailedLoad() -> bool
	{
		auto const directory = GetDefaultContext()->dataDirectory;
		GetDefaultContext()->dataDirectory = directory + "missing/";
		auto const failed = 0 == EsriProjectionEngine::FindHorizontalEpsgCodeByName("ETRS_1989_UTM_Zone_30N")
			&& 0 == EsriProjectionEngine::FindVerticalEpsgCodeByName("Alicante");
		GetDefaultContext()->dataDirectory = directory;

		std::vector<int> horizontal(8);
		std::vector<int> vertical(8);
		{
			std::vector<std::jthread> threads;
			for (size_t i = 0; i < horizontal.size(); ++i)
				threads.emplace_back([&, i]
				{
					horizontal[i] = EsriProjectionEngine::FindHorizontalEpsgCodeByName("ETRS_1989_UTM_Zone_30N");
					vertical[i] = EsriProjectionEngine::FindVerticalEpsgCodeByName("Alicante");
				});
		}
		auto loaded = true;
		for (size_t i = 0; i < horizontal.size(); ++i)
			loaded = loaded && 25830 == horizontal[i] && 5782 == vertical[i];
		return failed && loaded;
	}
}

TEST(EsriTablesDeathTest, FailedLoadIsRetriedOnTheNextCall)
{
	if (!TablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt or pe_list_vertcs.csv not installed in the data directory";

	GTEST_FLAG_SET(death_test_style, "threadsafe");
	EXPECT_EXIT(std::exit(LookUpAfterAFailedLoad() ? 0 : 1), ::testing::ExitedWithCode(0), "");
}
