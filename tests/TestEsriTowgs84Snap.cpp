// CrsContext::snapEsriNameWithTowgs84: an ESRI GEOGCS/PROJCS without AUTHORITY whose datum carries TOWGS84 is
// resolved by its ESRI name only when the option is set, the policy is CatalogWins and the ellipsoid and prime
// meridian match the catalogue CRS.

#include "TestBase.h"
#include "../crskit/EsriProjectionEngine.h"

#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	struct ScopedContext
	{
		WktParsePolicy previousPolicy{ GetDefaultContext()->parsePolicy };
		bool previousSnap{ GetDefaultContext()->snapEsriNameWithTowgs84 };
		ScopedContext(WktParsePolicy policy, bool snap)
		{
			GetDefaultContext()->parsePolicy = policy;
			GetDefaultContext()->snapEsriNameWithTowgs84 = snap;
		}
		~ScopedContext()
		{
			GetDefaultContext()->parsePolicy = previousPolicy;
			GetDefaultContext()->snapEsriNameWithTowgs84 = previousSnap;
		}
	};

	auto Geogcs(std::string const& semiMajorAxis, std::string const& primeMeridian) -> std::string
	{
		return "GEOGCS[\"GCS_European_1950\",DATUM[\"D_European_1950\",SPHEROID[\"International_1924\"," + semiMajorAxis +
			",297.0],TOWGS84[-87,-98,-121,0,0,0,0]],PRIMEM[\"Greenwich\"," + primeMeridian + "],UNIT[\"Degree\",0.0174532925199433]]";
	}

	auto Projcs(std::string const& semiMajorAxis) -> std::string
	{
		return "PROJCS[\"ED_1950_UTM_Zone_30N\"," + Geogcs(semiMajorAxis, "0.0") +
			",PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],"
			"PARAMETER[\"Central_Meridian\",-3.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";
	}

	// The ESRI table WKT of 'code' with a TOWGS84 node appended to its datum.
	auto EsriWithTowgs84(int code) -> std::string
	{
		auto wkt = EsriProjectionEngine::HorizontalWkt(code, "", false);
		auto const position = wkt.find("]],PRIMEM");
		if (std::string::npos == position)
			return {};
		return wkt.replace(position, 2, "],TOWGS84[1,2,3,0,0,0,0]]");
	}

	auto EsriTablesInstalled() -> bool
	{
		return !EsriProjectionEngine::NameFromHorizontalEpsgCode(4230).empty();
	}
}

TEST(EsriTowgs84Snap, GeographicIsKeptAsWrittenByDefault)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::CatalogWins, false };
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(Geogcs("6378388.0", "0.0"));
	EXPECT_EQ(0, cs->GetAuthorityCode());
	EXPECT_EQ("GCS_European_1950", cs->GetName());
}

TEST(EsriTowgs84Snap, GeographicResolvesByNameWhenTheEllipsoidMatches)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(Geogcs("6378388.0", "0.0"));
	EXPECT_EQ(4230, cs->GetAuthorityCode());
	EXPECT_EQ("ED50", cs->GetName());
}

TEST(EsriTowgs84Snap, GeographicWithAnotherEllipsoidIsKeptAsWritten)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(Geogcs("6378137.0", "0.0"));
	EXPECT_EQ(0, cs->GetAuthorityCode());
	EXPECT_EQ("GCS_European_1950", cs->GetName());
}

TEST(EsriTowgs84Snap, GeographicWithAnotherPrimeMeridianIsKeptAsWritten)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(Geogcs("6378388.0", "2.337229166666667"));
	EXPECT_EQ(0, cs->GetAuthorityCode());
}

TEST(EsriTowgs84Snap, TextWinsIgnoresTheOption)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::TextWins, true };
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(Geogcs("6378388.0", "0.0"));
	EXPECT_EQ(0, cs->GetAuthorityCode());
}

TEST(EsriTowgs84Snap, ProjectedResolvesByNameWhenTheEllipsoidMatches)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	{
		ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
		auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(Projcs("6378388.0"));
		EXPECT_EQ(23030, cs->GetAuthorityCode());
		EXPECT_EQ("ED50 / UTM zone 30N", cs->GetName());
	}
	{
		ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
		EXPECT_EQ(0, GetCoordinateSystemFactory()->CreateFromWkt(Projcs("6378137.0"))->GetAuthorityCode());
	}
	{
		ScopedContext const scoped{ WktParsePolicy::CatalogWins, false };
		EXPECT_EQ(0, GetCoordinateSystemFactory()->CreateFromWkt(Projcs("6378388.0"))->GetAuthorityCode());
	}
}

TEST(EsriTowgs84Snap, CatalogueEntriesWithRoundedOrNonGreenwichValuesResolve)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
	// NTF (Paris): ESRI writes the prime meridian in degrees although the GEOGCS unit is the grad.
	// Kalianpur 1880: ESRI rounds the semi-major axis to 1 cm and the inverse flattening to 4 decimals.
	// NTF (Paris) / Lambert zone II: the projected CRS on that base.
	for (auto const code : { 4807, 4243, 27572 })
	{
		auto const wkt = EsriWithTowgs84(code);
		ASSERT_FALSE(wkt.empty()) << code;
		EXPECT_EQ(code, GetCoordinateSystemFactory()->CreateFromWkt(wkt)->GetAuthorityCode()) << code;
	}
}

TEST(EsriTowgs84Snap, GeographicWithAnotherAngularUnitIsKeptAsWritten)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
	auto wkt = Geogcs("6378388.0", "0.0");
	wkt.replace(wkt.find("UNIT[\"Degree\",0.0174532925199433]"), std::string{ "UNIT[\"Degree\",0.0174532925199433]" }.size(), "UNIT[\"Radian\",1.0]");
	EXPECT_EQ(0, GetCoordinateSystemFactory()->CreateFromWkt(wkt)->GetAuthorityCode());
}

TEST(EsriTowgs84Snap, NameOfAnotherTypeIsBuiltFromTheText)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
	auto geogcs = Geogcs("6378388.0", "0.0");
	geogcs.replace(geogcs.find("GCS_European_1950"), std::string{ "GCS_European_1950" }.size(), "ED_1950_UTM_Zone_30N");
	EXPECT_EQ(0, GetCoordinateSystemFactory()->CreateFromWkt(geogcs)->GetAuthorityCode());

	auto projcs = Projcs("6378388.0");
	projcs.replace(projcs.find("ED_1950_UTM_Zone_30N"), std::string{ "ED_1950_UTM_Zone_30N" }.size(), "GCS_European_1950");
	EXPECT_EQ(0, GetCoordinateSystemFactory()->CreateFromWkt(projcs)->GetAuthorityCode());
}

TEST(EsriTowgs84Snap, ProjectedOnAnotherGeographicIsKeptAsWritten)
{
	if (!EsriTablesInstalled())
		GTEST_SKIP() << "esri_epsg.wkt not installed in the data directory";
	ScopedContext const scoped{ WktParsePolicy::CatalogWins, true };
	auto wkt = Projcs("6378388.0");
	wkt.replace(wkt.find("GCS_European_1950"), std::string{ "GCS_European_1950" }.size(), "GCS_Some_Other");
	EXPECT_EQ(0, GetCoordinateSystemFactory()->CreateFromWkt(wkt)->GetAuthorityCode());
}