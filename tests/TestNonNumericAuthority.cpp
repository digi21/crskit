// An AUTHORITY/ID whose code is not a number (IGNF:LAMB93) is read as a CRS without an EPSG code.

#include "TestBase.h"

#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	struct ScopedContext
	{
		CrsAliases previousAliases{ GetDefaultContext()->aliases };
		WktParsePolicy previousPolicy{ GetDefaultContext()->parsePolicy };
		ScopedContext(CrsAliases const& aliases, WktParsePolicy policy)
		{
			GetDefaultContext()->aliases = aliases;
			GetDefaultContext()->parsePolicy = policy;
		}
		~ScopedContext()
		{
			GetDefaultContext()->aliases = previousAliases;
			GetDefaultContext()->parsePolicy = previousPolicy;
		}
	};

	// The official WKT of EPSG 2154 (RGF93 / Lambert-93) with its outermost code replaced by IGNF:LAMB93.
	auto Lambert93AsIgnf(WktVersion version) -> std::string
	{
		auto wkt = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(2154)->GetWkt(version);
		std::string const id{ IsWkt2(version) ? "ID[\"EPSG\",2154]" : "AUTHORITY[\"EPSG\",\"2154\"]" };
		std::string const ignf{ IsWkt2(version) ? "ID[\"IGNF\",\"LAMB93\"]" : "AUTHORITY[\"IGNF\",\"LAMB93\"]" };
		wkt.replace(wkt.rfind(id), id.size(), ignf);
		return wkt;
	}
}

TEST(NonNumericAuthority, IsReadAsACrsWithoutEpsgCode)
{
	CrsAliases withAliases;
	withAliases.AddCrs(990001, 5773);

	for (auto const& aliases : { CrsAliases{}, withAliases })
		for (auto const policy : { WktParsePolicy::TextWins, WktParsePolicy::CatalogWins })
			for (auto const version : { WktVersion::Wkt1, WktVersion::Wkt2_2019 })
			{
				ScopedContext const scoped{ aliases, policy };
				std::shared_ptr<CoordinateSystem> cs;
				ASSERT_NO_THROW(cs = GetCoordinateSystemFactory()->CreateFromWkt(Lambert93AsIgnf(version)));
				EXPECT_EQ(0, cs->GetAuthorityCode());
				EXPECT_NE("EPSG", cs->GetAuthority());
				EXPECT_EQ(2, cs->GetDimension());
			}
}
