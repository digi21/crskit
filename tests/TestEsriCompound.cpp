// ESRI writes a compound CRS as a PROJCS followed by a VERTCS. Read back, it is the compound of both.

#include "TestBase.h"
#include "../crskit/EsriProjectionEngine.h"

#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	struct ScopedPolicy
	{
		WktParsePolicy previous{ GetDefaultContext()->parsePolicy };
		explicit ScopedPolicy(WktParsePolicy policy) { GetDefaultContext()->parsePolicy = policy; }
		~ScopedPolicy() { GetDefaultContext()->parsePolicy = previous; }
	};
}

// The VERTCS of the ESRI table carries no EPSG code; it is recognised by its text.
TEST(EsriCompound, ProjectedFollowedByVerticalIsACompound)
{
	auto const wkt = EsriProjectionEngine::CompoundWkt(25830, 5782, "");
	if (wkt.empty())
		GTEST_SKIP() << "esri_epsg.wkt or pe_list_vertcs.csv not installed in the data directory";
	ASSERT_NE(std::string::npos, wkt.find(",VERTCS["));
	ASSERT_EQ(std::string::npos, wkt.substr(wkt.find(",VERTCS[")).find("AUTHORITY"));

	for (auto const policy : { WktParsePolicy::TextWins, WktParsePolicy::CatalogWins })
	{
		ScopedPolicy const scoped{ policy };
		auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
		EXPECT_EQ(3, cs->GetDimension()) << static_cast<int>(policy);
		EXPECT_EQ("ETRS89 / UTM zone 30N + Alicante height", cs->GetName()) << static_cast<int>(policy);
	}
}
