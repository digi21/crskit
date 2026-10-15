// Geocentric to geocentric through the EPSG catalogue: the operation between the two CRS is applied.

#include "TestBase.h"

#include <vector>

using namespace CrsKit;

namespace
{
	auto Official(int code) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code); }

	auto ExpectNear(std::vector<double> const& expected, std::vector<double> const& actual) -> void
	{
		ASSERT_EQ(expected.size(), actual.size());
		for (size_t i = 0; i < expected.size(); ++i)
			EXPECT_NEAR(expected[i], actual[i], 1e-6);
	}
}

// ONGD14 -> WGS 84 has a single catalogued operation, EPSG 7377.
TEST(GeocentricOperations, SingleCatalogueOperationBothWays)
{
	std::vector<double> const point{ 3800000.0, 4900000.0, 2500000.0 };
	auto const operation = GetCoordinateTransformationAuthorityFactory()->CreateFromTransformationCode(7377)->GetMathTransform();
	auto const shifted = operation->Transform(point);
	EXPECT_GT(std::abs(shifted[0] - point[0]) + std::abs(shifted[1] - point[1]) + std::abs(shifted[2] - point[2]), 0.1);

	for (auto const viaAuthorityFactory : { true, false })
	{
		auto const create = [&](int source, int target) -> std::shared_ptr<CoordinateTransformations::ICoordinateTransformation>
		{
			if (viaAuthorityFactory)
				return GetCoordinateTransformationAuthorityFactory()->CreateFromCoordinateSystems(Official(source), Official(target));
			return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(Official(source), Official(target));
		};
		auto const direct = create(7371, 4978);
		EXPECT_EQ("7377", direct->GetAuthorityCode());
		ExpectNear(shifted, direct->GetMathTransform()->Transform(point));

		auto const inverse = create(4978, 7371);
		EXPECT_EQ("7377", inverse->GetAuthorityCode());
		ExpectNear(point, inverse->GetMathTransform()->Transform(shifted));
	}
}
