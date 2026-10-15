// CrsContext::aliases: caller-supplied CRS and operation equivalences resolve as the official catalogue
// entry on every entry path (factory by code, WKT1 with and without AUTHORITY under both parse policies,
// WKT2, compound CRS, ESRI WKT, operation codes and selectOperation). Codes 990000-990999 and the
// "MyApp" authority are invented; none of them is in the catalogue.

#include "TestBase.h"

#include <format>
#include <stdexcept>
#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr int kEgm96Height = 5773;
	constexpr int kNavd88Height = 5703;
	constexpr int kNavd88HeightFtUs = 6360;
	constexpr int kWgs84Utm30N = 32630;
	constexpr int kEtrs89Utm30N = 25830;
	constexpr int kEtrs89 = 4258;
	constexpr int kWgs84_3D = 4979;

	constexpr int kCustomVertical = 990001;
	constexpr int kCustomCompound = 990002;
	constexpr int kCustomProjected = 990010;
	constexpr int kCustomGeographic = 990011;
	constexpr int kCustomGeocentric = 990012;

	// Replaces the default context's aliases and parse policy for the scope of a test.
	struct ScopedAliases
	{
		CrsAliases previousAliases{ GetDefaultContext()->aliases };
		WktParsePolicy previousPolicy{ GetDefaultContext()->parsePolicy };
		explicit ScopedAliases(CrsAliases const& aliases, WktParsePolicy policy = WktParsePolicy::TextWins)
		{
			GetDefaultContext()->aliases = aliases;
			GetDefaultContext()->parsePolicy = policy;
		}
		~ScopedAliases()
		{
			GetDefaultContext()->aliases = previousAliases;
			GetDefaultContext()->parsePolicy = previousPolicy;
		}
	};

	auto VerticalAliases() -> CrsAliases
	{
		CrsAliases aliases;
		aliases.AddCrs(kCustomVertical, kEgm96Height);
		aliases.AddCrsName("My local height", kEgm96Height);
		aliases.AddCrs("MyApp", 17, kEgm96Height);
		return aliases;
	}

	auto Official(int code) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code);
	}

	auto DatumName(std::shared_ptr<CoordinateSystem> const& cs) -> std::string
	{
		auto const vertical = std::dynamic_pointer_cast<VerticalCoordinateSystem>(cs);
		if (!vertical)
			return {};
		return std::dynamic_pointer_cast<CoordinateSystems::IInfo>(vertical->GetVerticalDatum())->GetName();
	}

	// The CRS carries the official code, name and datum, and emits the official WKT1 and WKT2.
	auto ExpectOfficial(std::shared_ptr<CoordinateSystem> const& cs, int code) -> void
	{
		ASSERT_NE(nullptr, cs);
		auto const official = Official(code);
		EXPECT_EQ("EPSG", cs->GetAuthority());
		EXPECT_EQ(code, cs->GetAuthorityCode());
		EXPECT_EQ(official->GetName(), cs->GetName());
		EXPECT_EQ(DatumName(official), DatumName(cs));
		EXPECT_EQ(official->GetWkt(WktVersion::Wkt1), cs->GetWkt(WktVersion::Wkt1));
		EXPECT_EQ(official->GetWkt(WktVersion::Wkt2_2019), cs->GetWkt(WktVersion::Wkt2_2019));
	}

	auto FromWkt(std::string const& wkt) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemFactory()->CreateFromWkt(wkt);
	}

	const char* kVerticalWithCode =
		R"WKT(VERT_CS["My local height",VERT_DATUM["My local datum",2005],UNIT["metre",1],AXIS["Up",UP],AUTHORITY["EPSG","990001"]])WKT";
	const char* kVerticalWithoutCode =
		R"WKT(VERT_CS["  MY_LOCAL__height ",VERT_DATUM["My local datum",2005],UNIT["metre",1],AXIS["Up",UP]])WKT";
	const char* kVerticalWithOtherAuthority =
		R"WKT(VERT_CS["Another name",VERT_DATUM["My local datum",2005],UNIT["metre",1],AXIS["Up",UP],AUTHORITY["myapp","17"]])WKT";
	const char* kVerticalWkt2WithId =
		R"WKT(VERTCRS["Another name",VDATUM["My local datum"],CS[vertical,1],AXIS["gravity-related height (H)",up,LENGTHUNIT["metre",1]],ID["EPSG",990001]])WKT";
	const char* kVerticalWkt2WithoutId =
		R"WKT(VERTCRS["My local height",VDATUM["My local datum"],CS[vertical,1],AXIS["gravity-related height (H)",up,LENGTHUNIT["metre",1]]])WKT";

	// The official WKT of 'code' with its name and outermost AUTHORITY/ID code replaced.
	auto Renamed(int code, WktVersion version, std::string const& name, int newCode) -> std::string
	{
		auto const official = Official(code);
		auto wkt = official->GetWkt(version);
		auto const quotedName = std::format("\"{}\"", official->GetName());
		wkt.replace(wkt.find(quotedName), quotedName.size(), std::format("\"{}\"", name));
		auto const id = IsWkt2(version) ? std::format("ID[\"EPSG\",{}]", code) : std::format("AUTHORITY[\"EPSG\",\"{}\"]", code);
		auto const newId = IsWkt2(version) ? std::format("ID[\"EPSG\",{}]", newCode) : std::format("AUTHORITY[\"EPSG\",\"{}\"]", newCode);
		wkt.replace(wkt.rfind(id), id.size(), newId);
		return wkt;
	}
}

TEST(CrsAliases, NormalizeNameFoldsAsciiCaseUnderscoresAndSpaces)
{
	EXPECT_EQ("my local height", CrsAliases::NormalizeName("  MY_LOCAL__height \t"));
	EXPECT_EQ("a b", CrsAliases::NormalizeName("a _ b"));
	EXPECT_EQ("", CrsAliases::NormalizeName(" _ "));
	// Bytes >= 0x80 are compared as they are: an UTF-8 accented letter keeps its case and its accent.
	EXPECT_EQ("altura \xC3\x89", CrsAliases::NormalizeName("Altura \xC3\x89"));
	EXPECT_NE(CrsAliases::NormalizeName("altura \xC3\xA9"), CrsAliases::NormalizeName("altura \xC3\x89"));
	EXPECT_NE(CrsAliases::NormalizeName("altura e"), CrsAliases::NormalizeName("altura \xC3\xA9"));
}

TEST(CrsAliases, LookupsAndChainsAreRejected)
{
	CrsAliases aliases;
	EXPECT_TRUE(aliases.IsEmpty());
	aliases.AddCrs(kCustomVertical, kEgm96Height);
	aliases.AddCrs("MyApp", 17, kEgm96Height);
	aliases.AddCrsName("My local height", kEgm96Height);
	aliases.AddOperation(990100, 1149);

	EXPECT_EQ(kEgm96Height, aliases.FindCrs("epsg", kCustomVertical));
	EXPECT_EQ(kEgm96Height, aliases.FindCrs("MYAPP", 17));
	EXPECT_FALSE(aliases.FindCrs("EPSG", 17).has_value());
	EXPECT_EQ(kEgm96Height, aliases.FindCrsByName("my_local_height"));
	EXPECT_EQ(kEgm96Height, aliases.ResolveCrs(kCustomVertical));
	EXPECT_EQ(4326, aliases.ResolveCrs(4326));
	EXPECT_EQ(1149, aliases.ResolveOperation(990100));
	EXPECT_EQ(1150, aliases.ResolveOperation(1150));

	// A target cannot be a source, nor a source a target.
	EXPECT_THROW(aliases.AddCrs(kEgm96Height, kNavd88Height), std::invalid_argument);
	EXPECT_THROW(aliases.AddCrs(990003, kCustomVertical), std::invalid_argument);
	EXPECT_THROW(aliases.AddCrsName("Other", kCustomVertical), std::invalid_argument);
	EXPECT_THROW(aliases.AddOperation(1149, 1150), std::invalid_argument);
	EXPECT_THROW(aliases.AddOperation(990101, 990100), std::invalid_argument);
	EXPECT_THROW(aliases.AddCrs(990004, 0), std::invalid_argument);
	EXPECT_THROW(aliases.AddCrsName(" _ ", kEgm96Height), std::invalid_argument);

	// A copy keeps its tables when the original changes.
	auto const copy = aliases;
	aliases.Clear();
	EXPECT_TRUE(aliases.IsEmpty());
	EXPECT_EQ(kEgm96Height, copy.ResolveCrs(kCustomVertical));
}

TEST(CrsAliases, FactoryByCodeBuildsTheOfficialCrs)
{
	ScopedAliases _{ VerticalAliases() };

	ExpectOfficial(GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(kCustomVertical), kEgm96Height);
	ExpectOfficial(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(kCustomVertical), kEgm96Height);
	EXPECT_EQ(GetCoordinateSystemAuthorityFactory()->GetKindOfCoordinateSystem(kEgm96Height), GetCoordinateSystemAuthorityFactory()->GetKindOfCoordinateSystem(kCustomVertical));
}

TEST(CrsAliases, Wkt1VerticalResolvesUnderBothPolicies)
{
	for (auto const policy : { WktParsePolicy::TextWins, WktParsePolicy::CatalogWins })
	{
		ScopedAliases _{ VerticalAliases(), policy };

		ExpectOfficial(FromWkt(kVerticalWithCode), kEgm96Height);
		ExpectOfficial(FromWkt(kVerticalWithoutCode), kEgm96Height);
		ExpectOfficial(FromWkt(kVerticalWithOtherAuthority), kEgm96Height);
	}
}

TEST(CrsAliases, Wkt2VerticalResolvesByIdAndByName)
{
	ScopedAliases _{ VerticalAliases() };

	ExpectOfficial(FromWkt(kVerticalWkt2WithId), kEgm96Height);
	ExpectOfficial(FromWkt(kVerticalWkt2WithoutId), kEgm96Height);
}

TEST(CrsAliases, WithoutAliasesTheWktIsBuiltFromTheText)
{
	ScopedAliases _{ CrsAliases{} };

	auto const withCode = FromWkt(kVerticalWithCode);
	EXPECT_EQ("My local height", withCode->GetName());
	EXPECT_EQ(kCustomVertical, withCode->GetAuthorityCode());
	EXPECT_EQ("My local datum", DatumName(withCode));

	auto const withoutCode = FromWkt(kVerticalWithoutCode);
	EXPECT_EQ("  MY_LOCAL__height ", withoutCode->GetName());
	EXPECT_EQ(0, withoutCode->GetAuthorityCode());

	EXPECT_THROW(static_cast<void>(GetCoordinateSystemAuthorityFactory()->CreateVerticalCoordinateSystem(kCustomVertical)), AuthorityCodeNotFoundException);
}

TEST(CrsAliases, Wkt1CompoundReplacesTheVerticalAndRebuildsItsName)
{
	ScopedAliases _{ VerticalAliases() };

	auto const wkt = std::format(R"WKT(COMPD_CS["Old compound name",{},{},AUTHORITY["EPSG","{}"]])WKT",
		Official(kWgs84Utm30N)->GetWkt(WktVersion::Wkt1), kVerticalWithCode, kCustomCompound);
	auto const compound = std::dynamic_pointer_cast<CompoundCoordinateSystem>(FromWkt(wkt));
	ASSERT_NE(nullptr, compound);

	ExpectOfficial(std::dynamic_pointer_cast<CoordinateSystem>(compound->GetTailCS()), kEgm96Height);
	auto const official = GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(kWgs84Utm30N, kEgm96Height);
	EXPECT_EQ(official->GetName(), compound->GetName());
	EXPECT_EQ(0, compound->GetAuthorityCode());
	EXPECT_EQ(std::string::npos, compound->GetWkt(WktVersion::Wkt1).find("Old compound name"));
	EXPECT_EQ(std::string::npos, compound->GetWkt(WktVersion::Wkt1).find(std::to_string(kCustomCompound)));
}

TEST(CrsAliases, Wkt2CompoundReplacesTheVerticalAndRebuildsItsName)
{
	ScopedAliases _{ VerticalAliases() };

	auto const wkt = std::format(R"WKT(COMPOUNDCRS["Old compound name",{},{},ID["EPSG",{}]])WKT",
		Official(kWgs84Utm30N)->GetWkt(WktVersion::Wkt2_2019), kVerticalWkt2WithId, kCustomCompound);
	auto const compound = std::dynamic_pointer_cast<CompoundCoordinateSystem>(FromWkt(wkt));
	ASSERT_NE(nullptr, compound);

	ExpectOfficial(std::dynamic_pointer_cast<CoordinateSystem>(compound->GetTailCS()), kEgm96Height);
	auto const official = GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(kWgs84Utm30N, kEgm96Height);
	EXPECT_EQ(official->GetName(), compound->GetName());
	EXPECT_EQ(0, compound->GetAuthorityCode());
	EXPECT_EQ(std::string::npos, compound->GetWkt(WktVersion::Wkt2_2019).find("Old compound name"));
}

TEST(CrsAliases, CompoundWithoutAliasesKeepsItsNameAndCode)
{
	ScopedAliases _{ CrsAliases{} };

	auto const wkt = std::format(R"WKT(COMPOUNDCRS["Old compound name",{},{},ID["EPSG",{}]])WKT",
		Official(kWgs84Utm30N)->GetWkt(WktVersion::Wkt2_2019), kVerticalWkt2WithId, kCustomCompound);
	auto const compound = FromWkt(wkt);
	EXPECT_EQ(kCustomCompound, compound->GetAuthorityCode());
	EXPECT_NE(std::string::npos, compound->GetWkt(WktVersion::Wkt2_2019).find(std::format("ID[\"EPSG\",{}]", kCustomCompound)));
}

TEST(CrsAliases, EsriCompoundResolvesTheEmbeddedVertical)
{
	ScopedAliases _{ VerticalAliases() };

	auto const wkt =
		R"WKT(PROJCS["WGS_1984_UTM_Zone_30N",GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137.0,298.257223563]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]],PROJECTION["Transverse_Mercator"],PARAMETER["False_Easting",500000.0],PARAMETER["False_Northing",0.0],PARAMETER["Central_Meridian",-3.0],PARAMETER["Scale_Factor",0.9996],PARAMETER["Latitude_Of_Origin",0.0],UNIT["Meter",1.0],AUTHORITY["EPSG","32630"],VERTCS["My_local_height",VDATUM["My_local_datum"],PARAMETER["Vertical_Shift",0.0],PARAMETER["Direction",1.0],UNIT["Meter",1.0]])WKT";
	auto const compound = std::dynamic_pointer_cast<CompoundCoordinateSystem>(FromWkt(wkt));
	ASSERT_NE(nullptr, compound);
	ExpectOfficial(std::dynamic_pointer_cast<CoordinateSystem>(compound->GetTailCS()), kEgm96Height);
}

TEST(CrsAliases, HorizontalCrsResolveByCodeAndByName)
{
	CrsAliases aliases;
	aliases.AddCrs(kCustomProjected, kEtrs89Utm30N);
	aliases.AddCrs(kCustomGeographic, kEtrs89);
	aliases.AddCrs(kCustomGeocentric, 4936);
	aliases.AddCrsName("My projected", kEtrs89Utm30N);

	for (auto const policy : { WktParsePolicy::TextWins, WktParsePolicy::CatalogWins })
	{
		ScopedAliases _{ aliases, policy };

		for (auto const version : { WktVersion::Wkt1, WktVersion::Wkt2_2019 })
		{
			ExpectOfficial(FromWkt(Renamed(kEtrs89Utm30N, version, "Some projected", kCustomProjected)), kEtrs89Utm30N);
			ExpectOfficial(FromWkt(Renamed(kEtrs89Utm30N, version, "MY_PROJECTED", 990999)), kEtrs89Utm30N);
			ExpectOfficial(FromWkt(Renamed(kEtrs89, version, "Some geographic", kCustomGeographic)), kEtrs89);
			ExpectOfficial(FromWkt(Renamed(4936, version, "Some geocentric", kCustomGeocentric)), 4936);
		}
		ExpectOfficial(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(kCustomProjected), kEtrs89Utm30N);
	}
}

TEST(CrsAliases, VerticalTransformationMatchesTheOfficialOne)
{
	CrsAliases aliases;
	aliases.AddCrs(kCustomVertical, kNavd88Height);
	ScopedAliases _{ aliases };

	auto const target = Official(kNavd88HeightFtUs);
	auto const viaAlias = TestCrsKit::BuildVerticalTransform(FromWkt(kVerticalWithCode), target);
	auto const official = TestCrsKit::BuildVerticalTransform(Official(kNavd88Height), target);

	auto const a = viaAlias->Transform({ 100.0 });
	auto const b = official->Transform({ 100.0 });
	ASSERT_EQ(b.size(), a.size());
	for (size_t i = 0; i < a.size(); ++i)
		EXPECT_DOUBLE_EQ(b[i], a[i]);
	EXPECT_NEAR(328.0833, b.back(), 1e-3);
}

// Needs the EGM96 geoid grid of EPSG 10084 in the data directory; skipped when it is missing.
TEST(CrsAliases, CompoundGeoidTransformationMatchesTheOfficialOne)
{
	ScopedAliases _{ VerticalAliases() };

	auto const target = Official(kWgs84_3D);
	auto const compound = [](int vertical)
	{
		return std::dynamic_pointer_cast<CoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(kWgs84Utm30N, vertical));
	};

	try
	{
		auto const viaAlias = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(compound(kCustomVertical), target);
		auto const official = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(compound(kEgm96Height), target);

		std::vector<double> const point{ 500000.0, 4400000.0, 0.0 };
		auto const a = viaAlias->GetMathTransform()->Transform(point);
		auto const b = official->GetMathTransform()->Transform(point);
		ASSERT_EQ(3u, a.size());
		ASSERT_EQ(3u, b.size());
		for (size_t i = 0; i < 3; ++i)
			EXPECT_DOUBLE_EQ(b[i], a[i]);
		EXPECT_GT(std::abs(b[2]), 1.0);  // The geoid undulation was applied.
	}
	catch (GridFileNotFoundException const& e)
	{
		GTEST_SKIP() << e.what();
	}
}

// Needs the EGM96 geoid grid of EPSG 10084 in the data directory; skipped when it is missing.
TEST(CrsAliases, OperationCodeResolvesAsTheOfficialOperation)
{
	CrsAliases aliases;
	aliases.AddOperation(990100, 10084);
	ScopedAliases _{ aliases };

	try
	{
		auto const factory = GetCoordinateTransformationAuthorityFactory();
		auto const viaAlias = factory->CreateFromTransformationCode(990100);
		auto const official = factory->CreateFromTransformationCode(10084);

		EXPECT_EQ(official->GetName(), viaAlias->GetName());
		EXPECT_EQ("10084", viaAlias->GetAuthorityCode());
		std::vector<double> const point{ 40.0, -3.0, 100.0 };
		auto const a = viaAlias->GetMathTransform()->Transform(point);
		auto const b = official->GetMathTransform()->Transform(point);
		ASSERT_EQ(b.size(), a.size());
		for (size_t i = 0; i < a.size(); ++i)
			EXPECT_DOUBLE_EQ(b[i], a[i]);
	}
	catch (GridFileNotFoundException const& e)
	{
		GTEST_SKIP() << e.what();
	}
}

TEST(CrsAliases, SelectOperationMayReturnAnAlias)
{
	CrsAliases aliases;
	aliases.AddOperation(990101, 1133);
	ScopedAliases _{ aliases };

	auto const select = [](int code)
	{
		return CoordinateTransformationOptions{ .selectOperation = [code](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return code; } };
	};
	auto const source = Official(4230);
	auto const target = Official(4326);
	auto const viaAlias = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target, select(990101));
	auto const official = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target, select(1133));

	EXPECT_EQ(official->GetName(), viaAlias->GetName());
	EXPECT_EQ(official->GetAuthorityCode(), viaAlias->GetAuthorityCode());
	auto const a = viaAlias->GetMathTransform()->Transform({ 40.0, -3.0 });
	auto const b = official->GetMathTransform()->Transform({ 40.0, -3.0 });
	ASSERT_EQ(b.size(), a.size());
	for (size_t i = 0; i < a.size(); ++i)
		EXPECT_DOUBLE_EQ(b[i], a[i]);
}

// A CRS built from the text keeps its custom code; operation lookups by code still use the alias target.
TEST(CrsAliases, OperationLookupUsesTheAliasOfACrsBuiltBeforehand)
{
	auto const custom = [] { ScopedAliases _{ CrsAliases{} }; return FromWkt(kVerticalWithCode); }();
	ASSERT_EQ(kCustomVertical, custom->GetAuthorityCode());

	CrsAliases aliases;
	aliases.AddCrs(kCustomVertical, kEgm96Height);
	ScopedAliases _{ aliases };

	auto const geographic = std::dynamic_pointer_cast<GeographicCoordinateSystem>(Official(kWgs84_3D));
	auto const vertical = std::dynamic_pointer_cast<VerticalCoordinateSystem>(custom);
	try
	{
		auto const ct = GetCoordinateTransformationAuthorityFactory()->CreateCoordinateTransformBetweenGeographicVerticalSystems(geographic, vertical, false, CoordinateTransformationOptions{});
		ASSERT_NE(nullptr, ct);
		EXPECT_NE("0", ct->GetAuthorityCode());
	}
	catch (GridFileNotFoundException const& e)
	{
		GTEST_SKIP() << e.what();
	}
}

namespace
{
	// The operation the factory picks between two CRS is the alias target of the single candidate.
	auto ExpectAliasedSingleCandidate(int sourceCode, int targetCode, int candidate, int replacement) -> void
	{
		CrsAliases aliases;
		aliases.AddOperation(candidate, replacement);
		ScopedAliases _{ aliases };

		auto const factory = GetCoordinateTransformationAuthorityFactory();
		auto const ct = factory->CreateFromCoordinateSystems(Official(sourceCode), Official(targetCode));
		auto const official = factory->CreateFromTransformationCode(replacement);
		EXPECT_EQ(std::to_string(replacement), ct->GetAuthorityCode());
		EXPECT_EQ(official->GetName(), ct->GetName());
		EXPECT_EQ(official->GetAreaOfUse(), ct->GetAreaOfUse());
	}
}

TEST(CrsAliases, SingleCandidateOperationResolvesItsAlias)
{
	// Vertical: NZVD2009 -> One Tree Point (only EPSG 4441), direct and inverse.
	ExpectAliasedSingleCandidate(4440, 5767, 4441, 4442);
	ExpectAliasedSingleCandidate(5767, 4440, 4441, 4442);
	// Geocentric: ONGD14 -> WGS 84 (only EPSG 7377), direct and inverse.
	ExpectAliasedSingleCandidate(7371, 4978, 7377, 7666);
	ExpectAliasedSingleCandidate(4978, 7371, 7377, 7666);
}

TEST(CrsAliases, AddingAnExistingSourceReplacesItsTarget)
{
	CrsAliases crs;
	crs.AddCrs(kEgm96Height, kEgm96Height);
	EXPECT_NO_THROW(crs.AddCrs(kEgm96Height, kNavd88Height));
	EXPECT_EQ(kNavd88Height, crs.ResolveCrs(kEgm96Height));

	CrsAliases identity;
	identity.AddCrs(kCustomVertical, kEgm96Height);
	EXPECT_NO_THROW(identity.AddCrs(kCustomVertical, kCustomVertical));
	EXPECT_EQ(kCustomVertical, identity.ResolveCrs(kCustomVertical));

	CrsAliases operations;
	operations.AddOperation(1149, 1149);
	EXPECT_NO_THROW(operations.AddOperation(1149, 1150));
	EXPECT_EQ(1150, operations.ResolveOperation(1149));

	CrsAliases operationIdentity;
	operationIdentity.AddOperation(990100, 1149);
	EXPECT_NO_THROW(operationIdentity.AddOperation(990100, 990100));
	EXPECT_EQ(990100, operationIdentity.ResolveOperation(990100));

	// Chains through another source are still rejected.
	CrsAliases chained;
	chained.AddCrs(kCustomVertical, kEgm96Height);
	EXPECT_THROW(chained.AddCrs(kEgm96Height, kNavd88Height), std::invalid_argument);
	EXPECT_THROW(chained.AddCrs(990003, kCustomVertical), std::invalid_argument);
	chained.AddOperation(990100, 1149);
	EXPECT_THROW(chained.AddOperation(1149, 1150), std::invalid_argument);
	EXPECT_THROW(chained.AddOperation(990101, 990100), std::invalid_argument);
}
