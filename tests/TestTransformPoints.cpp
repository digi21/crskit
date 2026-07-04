// Verifies the batch API IMathTransform::TransformPoints (a flat buffer of N points)
// against the per-point API Transform(): they must give exactly the same result.

#include "TestBase.h"

using namespace TestCrsKit;
using namespace CrsKit;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	// Affine 2D->2D that swaps X and Y (elt_0_1 = elt_1_0 = 1).
	const char* kAffineSwapXy = R"WKT(PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]])WKT";

	// Chain: Affine (swaps axes) + Transverse_Mercator. Exercises the batch-through of
	// ConcatenatedTransform + Affine's in-place + the default TransformPoints of the projection.
	const char* kConcatAffineTm = R"WKT(CONCAT_MT[PARAM_MT["Affine",PARAMETER["num_row",3],PARAMETER["num_col",3],PARAMETER["elt_0_0",0],PARAMETER["elt_0_1",1],PARAMETER["elt_0_2",0],PARAMETER["elt_1_0",1],PARAMETER["elt_1_1",0],PARAMETER["elt_1_2",0],PARAMETER["elt_2_0",0],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1]],PARAM_MT["Transverse_Mercator",PARAMETER["semi_major",6378137],PARAMETER["semi_minor",6356752.314247833],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",2.999999999999997],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0]]])WKT";

	void CheckBatchMatchesPerPoint(const char* wkt, std::vector<std::vector<double>> const& points)
	{
		auto const t = GetMathTransformFactory()->CreateFromWkt(wkt);

		std::vector<double> expected;
		for (auto const& point : points)
		{
			auto const transformed = t->Transform(point);
			expected.insert(expected.end(), transformed.begin(), transformed.end());
		}

		std::vector<double> flat;
		for (auto const& point : points)
			flat.insert(flat.end(), point.begin(), point.end());

		auto const batch = t->TransformPoints(flat);

		ASSERT_EQ(expected.size(), batch.size());
		for (size_t k = 0; k < expected.size(); ++k)
			EXPECT_DOUBLE_EQ(expected[k], batch[k]);
	}
}

TEST(TransformPoints, BatchMatchesPerPoint)
{
	auto const mtf = GetMathTransformFactory();
	auto const t = mtf->CreateFromWkt(kAffineSwapXy);

	ASSERT_EQ(2, t->GetSourceDimension());
	ASSERT_EQ(2, t->GetTargetDimension());

	std::vector<std::vector<double>> const points{ {1.0, 2.0}, {3.0, 4.0}, {-5.0, 6.5} };

	// Reference result: per-point API.
	std::vector<double> expected;
	for (auto const& point : points)
	{
		auto const transformed = t->Transform(point);
		expected.insert(expected.end(), transformed.begin(), transformed.end());
	}

	// Batch API over a flat buffer.
	std::vector<double> flat;
	for (auto const& point : points)
		flat.insert(flat.end(), point.begin(), point.end());

	auto const batch = t->TransformPoints(flat);

	ASSERT_EQ(expected.size(), batch.size());
	for (size_t k = 0; k < expected.size(); ++k)
		EXPECT_DOUBLE_EQ(expected[k], batch[k]);

	// Sanity: the Affine swaps X<->Y.
	EXPECT_DOUBLE_EQ(2.0, batch[0]);
	EXPECT_DOUBLE_EQ(1.0, batch[1]);
}

TEST(TransformPoints, EmptyInputYieldsEmptyOutput)
{
	auto const mtf = GetMathTransformFactory();
	auto const t = mtf->CreateFromWkt(kAffineSwapXy);

	std::vector<double> const empty;
	EXPECT_TRUE(t->TransformPoints(empty).empty());
}

TEST(TransformPoints, ConcatenatedChainBatchMatchesPerPoint)
{
	// Geographic points (lat, lon) in decimal degrees.
	CheckBatchMatchesPerPoint(kConcatAffineTm, { {60.0, -2.0}, {20.0, -2.0}, {0.0, -2.0}, {60.0, 5.0} });
}
