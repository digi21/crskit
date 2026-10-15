// Compound CRS read from WKT1 (COMPD_CS with EPSG components, with and without an AUTHORITY of their
// own) transformed to a geographic 3D CRS and to another compound, under both parse policies.

#include "TestBase.h"

#include <format>
#include <string>
#include <vector>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr int kEtrs89Utm30N = 25830;
	constexpr int kEtrs89Utm29N = 25829;
	constexpr int kAlicanteHeight = 5782;
	constexpr int kEtrs89_3D = 4937;

	struct ScopedPolicy
	{
		WktParsePolicy previous{ GetDefaultContext()->parsePolicy };
		explicit ScopedPolicy(WktParsePolicy policy) { GetDefaultContext()->parsePolicy = policy; }
		~ScopedPolicy() { GetDefaultContext()->parsePolicy = previous; }
	};

	auto Official(int code) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code);
	}

	// COMPD_CS of the official WKT1 of both components; 'authority' is appended as the compound's last child.
	auto CompoundWkt(int horizontal, int vertical, std::string const& authority) -> std::string
	{
		return std::format("COMPD_CS[\"{} + {}\",{},{}{}]", Official(horizontal)->GetName(), Official(vertical)->GetName(),
			Official(horizontal)->GetWkt(WktVersion::Wkt1), Official(vertical)->GetWkt(WktVersion::Wkt1), authority);
	}

	auto Compound(int horizontal, int vertical, std::string const& authority) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemFactory()->CreateFromWkt(CompoundWkt(horizontal, vertical, authority));
	}

	auto Transform(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, std::vector<double> const& point) -> std::vector<double>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)->GetMathTransform()->Transform(point);
	}

	auto const kPolicies = { WktParsePolicy::TextWins, WktParsePolicy::CatalogWins };
	auto const kCompoundAuthorities = { std::string{}, std::string{ ",AUTHORITY[\"EPSG\",\"9505\"]" } };
	std::vector<double> const kProjectedPoint{ 440000.0, 4475000.0, 650.0 };
}

// Needs the REDNAP geoid of EPSG 9410 in the data directory; skipped when it is missing.
TEST(CompoundFromWkt, ToGeographic3DAndBack)
{
	try
	{
		// Without an AUTHORITY on the compound: the reference.
		auto const expected = Transform(Compound(kEtrs89Utm30N, kAlicanteHeight, {}), Official(kEtrs89_3D), kProjectedPoint);
		ASSERT_EQ(3u, expected.size());
		EXPECT_NEAR(40.4234689, expected[0], 1e-6);
		EXPECT_NEAR(-3.7072934, expected[1], 1e-6);
		EXPECT_GT(expected[2] - kProjectedPoint[2], 40.0);  // The geoid undulation was applied.

		for (auto const policy : kPolicies)
			for (auto const& authority : kCompoundAuthorities)
			{
				auto const context = std::format("policy {} authority '{}'", static_cast<int>(policy), authority);
				ScopedPolicy const scoped{ policy };
				auto const compound = Compound(kEtrs89Utm30N, kAlicanteHeight, authority);

				auto const geographic = Transform(compound, Official(kEtrs89_3D), kProjectedPoint);
				ASSERT_EQ(3u, geographic.size()) << context;
				for (size_t i = 0; i < 3; ++i)
					EXPECT_NEAR(expected[i], geographic[i], 1e-9) << context;

				auto const back = Transform(Official(kEtrs89_3D), compound, geographic);
				ASSERT_EQ(3u, back.size()) << context;
				for (size_t i = 0; i < 3; ++i)
					EXPECT_NEAR(kProjectedPoint[i], back[i], 1e-3) << context;
			}
	}
	catch (GridFileNotFoundException const& e)
	{
		GTEST_SKIP() << e.what();
	}
}

TEST(CompoundFromWkt, ToCompoundWithTheSameVertical)
{
	auto const horizontal = Transform(Official(kEtrs89Utm30N), Official(kEtrs89Utm29N), { kProjectedPoint[0], kProjectedPoint[1] });

	for (auto const policy : kPolicies)
		for (auto const& authority : kCompoundAuthorities)
		{
			auto const context = std::format("policy {} authority '{}'", static_cast<int>(policy), authority);
			ScopedPolicy const scoped{ policy };
			auto const result = Transform(Compound(kEtrs89Utm30N, kAlicanteHeight, authority), Compound(kEtrs89Utm29N, kAlicanteHeight, authority), kProjectedPoint);
			ASSERT_EQ(3u, result.size()) << context;
			EXPECT_NEAR(horizontal[0], result[0], 1e-6) << context;
			EXPECT_NEAR(horizontal[1], result[1], 1e-6) << context;
			EXPECT_DOUBLE_EQ(kProjectedPoint[2], result[2]) << context;
		}
}

// A vertical without an EPSG code and no resolveTransform: no vertical operation is found.
TEST(CompoundFromWkt, Geographic3DToCompoundWithAnUnknownVerticalThrows)
{
	auto const wkt = std::format("COMPD_CS[\"c\",{},VERT_CS[\"H\",VERT_DATUM[\"H\",2005],UNIT[\"metre\",1],AXIS[\"Up\",UP]]]", Official(kEtrs89Utm30N)->GetWkt(WktVersion::Wkt1));
	auto const compound = GetCoordinateSystemFactory()->CreateFromWkt(wkt);
	EXPECT_THROW(static_cast<void>(GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Official(kEtrs89_3D), compound)), TransformationNotFoundException);
}
