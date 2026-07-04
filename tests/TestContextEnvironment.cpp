// The library's mutable state lives in a single Environment object (the default CrsContext, EPSG
// provider and factories), reachable via the Get* facade and swappable as a unit with
// SetCurrentEnvironment. Not part of the GIGS conformance suite, so the suite name avoids the "Test5xxx" prefix.

#include "TestBase.h"

using namespace CrsKit;

// GetDefaultContext() returns the context held by the current Environment, not a separate global.
TEST(EnvironmentContext, GetDefaultContextComesFromCurrentEnvironment)
{
	auto const environment = GetCurrentEnvironment();
	ASSERT_NE(nullptr, environment);
	EXPECT_EQ(environment->DefaultContext, GetDefaultContext());
}

// The whole context can be replaced atomically and restored.
TEST(EnvironmentContext, SetCurrentEnvironmentSwapsAsAUnit)
{
	auto const original = GetCurrentEnvironment();

	auto replacement = std::make_shared<Environment>();
	replacement->DefaultContext = std::make_shared<CrsContext>();
	replacement->DefaultContext->dataDirectory = "swap-test-marker/";

	SetCurrentEnvironment(replacement);
	EXPECT_EQ(replacement, GetCurrentEnvironment());
	EXPECT_EQ("swap-test-marker/", GetDefaultContext()->dataDirectory);

	SetCurrentEnvironment(original);
	EXPECT_EQ(original, GetCurrentEnvironment());
}

namespace
{
	// A compound CRS with a known geographic head and a local (unknown, datum 32767) vertical tail, so the
	// createHorizontalWktIfVerticalLocal switch has something to act on.
	const char* kCompoundWithLocalVertical =
		R"WKT(COMPD_CS["c",GEOGCS["Test GCS",DATUM["Test Datum",SPHEROID["Test ellipsoid",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]],LOCAL_CS["LocalV",LOCAL_DATUM["LocalV",32767],UNIT["m",1.0],AXIS["Z", UP]]])WKT";
}

// GetWkt(CrsContext const&) drives emission entirely from the passed context: the same CRS emits a full
// COMPD_CS or a horizontal-only WKT depending on the context's createHorizontalWktIfVerticalLocal, with no
// dependence on the default context.
TEST(CrsContextEmission, PerCallContextDrivesCompoundLocalEmission)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kCompoundWithLocalVertical);

	CrsContext full{};
	full.createHorizontalWktIfVerticalLocal = false;
	EXPECT_TRUE(cs->GetWkt(full).starts_with("COMPD_CS["));

	CrsContext horizontalOnly{};
	horizontalOnly.createHorizontalWktIfVerticalLocal = true;
	EXPECT_TRUE(cs->GetWkt(horizontalOnly).starts_with("GEOGCS["));
}

// Emitting with an explicit context neither reads nor mutates the default context.
TEST(CrsContextEmission, PerCallContextDoesNotTouchDefault)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kCompoundWithLocalVertical);

	auto const defaultBefore = *GetDefaultContext();

	CrsContext flipped{};
	flipped.wktVersion = WktVersion::Wkt2_2019;
	flipped.createHorizontalWktIfVerticalLocal = !defaultBefore.createHorizontalWktIfVerticalLocal;
	(void)cs->GetWkt(flipped);

	EXPECT_EQ(defaultBefore.wktVersion, GetDefaultContext()->wktVersion);
	EXPECT_EQ(defaultBefore.createHorizontalWktIfVerticalLocal, GetDefaultContext()->createHorizontalWktIfVerticalLocal);
}

namespace
{
	// Restores the default context's parse policy on scope exit.
	struct ScopedParsePolicy
	{
		WktParsePolicy previous{ GetDefaultContext()->parsePolicy };
		explicit ScopedParsePolicy(WktParsePolicy policy) { GetDefaultContext()->parsePolicy = policy; }
		~ScopedParsePolicy() { GetDefaultContext()->parsePolicy = previous; }
	};

	// A WGS 84 geographic CRS whose name deliberately differs from the catalogue's ("WGS 84"), carrying a
	// real EPSG authority. The parse policy decides whether the object comes from the text or the catalogue.
	const char* kNamedWgs84WithEpsgAuthority =
		R"WKT(GEOGCS["My Custom WGS84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]])WKT";
}

// TextWins (default): build from the WKT pieces, keeping the WKT name and retaining the EPSG code only as
// metadata -- the AUTHORITY does not redirect to the catalogue.
TEST(CrsContextParse, TextWinsKeepsTheWktNameAndRetainsTheCode)
{
	ScopedParsePolicy _{ WktParsePolicy::TextWins };

	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kNamedWgs84WithEpsgAuthority);

	EXPECT_EQ("My Custom WGS84", cs->GetName());
	EXPECT_EQ(4326, cs->GetAuthorityCode());
}

// CatalogWins (legacy): the EPSG AUTHORITY redirects to the catalogue, so the parsed object carries the
// official name, not the one written in the WKT.
TEST(CrsContextParse, CatalogWinsSnapsToTheAuthorityName)
{
	ScopedParsePolicy _{ WktParsePolicy::CatalogWins };

	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kNamedWgs84WithEpsgAuthority);

	EXPECT_NE("My Custom WGS84", cs->GetName());
	EXPECT_EQ(4326, cs->GetAuthorityCode());
}

namespace
{
	// ESRI-flavoured (ArcGIS) geographic WKT: "GCS_"/"D_" names. With an EPSG authority code...
	const char* kEsriGeographicWithCode =
		R"WKT(GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137.0,298.257223563]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433],AUTHORITY["EPSG","4326"]])WKT";
	// ...and without one.
	const char* kEsriGeographicNoCode =
		R"WKT(GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137.0,298.257223563]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]])WKT";

	// ESRI stores a compound CRS as a VERTCS embedded in the PROJCS. Here both parts carry an EPSG code
	// (32630 = WGS 84 / UTM zone 30N, 5773 = EGM96 height).
	const char* kEsriCompoundWithCodes =
		R"WKT(PROJCS["WGS_1984_UTM_Zone_30N",GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137.0,298.257223563]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]],PROJECTION["Transverse_Mercator"],PARAMETER["False_Easting",500000.0],PARAMETER["False_Northing",0.0],PARAMETER["Central_Meridian",-3.0],PARAMETER["Scale_Factor",0.9996],PARAMETER["Latitude_Of_Origin",0.0],UNIT["Meter",1.0],AUTHORITY["EPSG","32630"],VERTCS["EGM96_geoid",VDATUM["EGM96_geoid"],PARAMETER["Vertical_Shift",0.0],PARAMETER["Direction",1.0],UNIT["Meter",1.0],AUTHORITY["EPSG","5773"]])WKT";
}

// ESRI WKT carrying an EPSG code snaps to the catalogue by that code even under the default TextWins
// policy, so the object gets the clean EPSG name instead of the ArcGIS "GCS_WGS_1984".
TEST(EsriParse, EsriWithEpsgCodeSnapsToCatalogueByCode)
{
	ScopedParsePolicy _{ WktParsePolicy::TextWins };

	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kEsriGeographicWithCode);

	EXPECT_EQ(4326, cs->GetAuthorityCode());
	EXPECT_NE("GCS_WGS_1984", cs->GetName());
}

// ESRI WKT without a code is built from its pieces (TextWins): the ESRI name is kept and nothing snaps it
// to the catalogue by name-matching.
TEST(EsriParse, EsriWithoutCodeIsBuiltFromPieces)
{
	ScopedParsePolicy _{ WktParsePolicy::TextWins };

	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kEsriGeographicNoCode);

	EXPECT_EQ("GCS_WGS_1984", cs->GetName());
	EXPECT_EQ(CoordinateSystems::UnknownAuthorityCode, cs->GetAuthorityCode());
}

// ESRI's compound (a VERTCS embedded in the PROJCS) with an EPSG code on each part is rebuilt from the
// catalogue as a proper compound CRS, so the vertical is no longer dropped.
TEST(EsriParse, EsriCompoundWithCodesRebuildsAsCompound)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kEsriCompoundWithCodes);

	auto const compound = std::dynamic_pointer_cast<CoordinateSystems::CompoundCoordinateSystem>(cs);
	ASSERT_NE(nullptr, compound);
	EXPECT_EQ(32630, compound->GetHeadCS()->GetAuthorityCode());
	EXPECT_EQ(5773, compound->GetTailCS()->GetAuthorityCode());
}
