// StaticCoordinateTransformationFactory::CreateFromCoordinateSystems between three-dimensional coordinate reference systems
// (CRS) when at least one horizontal component is local.

#include "TestBase.h"
#include "../crskit/IUtilities.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	auto CreateEpsgCoordinateSystem(int epsgCode) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(epsgCode);
	}

	auto CreateCompoundCrsWithLocalHorizontalAndVerticalComponents() -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetCompoundLocalWkt());
	}

	// Returns a compound CRS of ETRS89 / UTM zone 30N (European Terrestrial Reference System 1989, Universal Transverse
	// Mercator projection, zone 30N; European Petroleum Survey Group (EPSG) code 25830) and a local vertical CRS.
	auto CreateEtrs89UtmZone30NWithLocalVertical() -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetCompoundHorizontalPlusVerticalLocalWkt("ETRS89 / UTM zone 30N", CreateEpsgCoordinateSystem(25830)->GetWkt().c_str()));
	}

	auto ExpectPointCoordinatesWithinTolerance(std::vector<double> const& expectedPoint, std::vector<double> const& point, double tolerance) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerance) << coordinateIndex;
	}

	std::vector<double> const kTestPointCoordinates{ 440000.0, 4474000.0, 650.0 };
}

// With UnknownCrsPolicy::Identity, the transformation from a compound CRS of a local horizontal CRS and a local vertical
// CRS to a compound CRS of EPSG 25830 and a local vertical CRS receives three coordinates and returns them unchanged.
TEST(ThreeDimensionalCrsWithLocalHorizontalComponent, LocalHorizontalToProjectedWithIdentityPolicyReturnsThreeCoordinatesUnchanged)
{
	CoordinateTransformationOptions options;
	options.unknownCrsPolicy = UnknownCrsPolicy::Identity;
	auto const mathTransform = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCompoundCrsWithLocalHorizontalAndVerticalComponents(), CreateEtrs89UtmZone30NWithLocalVertical(), options)->GetMathTransform();
	EXPECT_EQ(3, mathTransform->GetSourceDimension());
	ExpectPointCoordinatesWithinTolerance(kTestPointCoordinates, mathTransform->Transform(kTestPointCoordinates), 0.0);
}

// With UnknownCrsPolicy::Reject, CreateFromCoordinateSystems throws std::runtime_error when asked to create a
// transformation from a compound CRS of a local horizontal CRS and a local vertical CRS to a compound CRS of EPSG 25830 and
// a local vertical CRS.
TEST(ThreeDimensionalCrsWithLocalHorizontalComponent, LocalHorizontalToProjectedWithRejectPolicyThrowsRuntimeError)
{
	CoordinateTransformationOptions options;
	options.unknownCrsPolicy = UnknownCrsPolicy::Reject;
	EXPECT_THROW(static_cast<void>(GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCompoundCrsWithLocalHorizontalAndVerticalComponents(), CreateEtrs89UtmZone30NWithLocalVertical(), options)), std::runtime_error);
}

// The transformation between two compound CRSs of a local horizontal CRS and a local vertical CRS receives three
// coordinates and returns them unchanged.
TEST(ThreeDimensionalCrsWithLocalHorizontalComponent, LocalHorizontalToLocalHorizontalReturnsThreeCoordinatesUnchanged)
{
	auto const mathTransform = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCompoundCrsWithLocalHorizontalAndVerticalComponents(), CreateCompoundCrsWithLocalHorizontalAndVerticalComponents())->GetMathTransform();
	EXPECT_EQ(3, mathTransform->GetSourceDimension());
	ExpectPointCoordinatesWithinTolerance(kTestPointCoordinates, mathTransform->Transform(kTestPointCoordinates), 0.0);
}

