// When several EPSG operations can serve the same pair of systems, the library must ask the caller
// which one to use (CoordinateTransformationOptions::selectOperation) and refuse to guess when there
// is nobody to ask. This covers the geoid path, where the choice picks the grid: EGM2008 height is
// served by two operations, a 1' grid (3859) and a 2.5' one (3858).

#include "TestBase.h"

#include "../crskit/GridFileNotFoundException.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr auto egm2008Height = 3855;
	constexpr auto wgs84 = 4979;   // WGS 84 (Lat, Lon, h): the catalogue maps it to EGM2008 height directly
	constexpr auto twd97 = 3823;   // TWD97 (Lat, Lon, h): no catalogued operation to EGM2008 height, so the
	                               // datum-equivalent geoid fallback takes over

	// Builds the transformation, capturing the candidates offered. The math transform loads the geoid
	// grid, which is not redistributable and may be absent: that failure comes AFTER the operation has
	// been chosen, which is what this file is about, so it is not a test failure.
	std::vector<int> CandidatesOfferedFor(int sourceCode, int chosen)
	{
		std::vector<int> offered;

		CoordinateTransformationOptions options;
		options.selectOperation = [&offered, chosen](std::string const&, std::string const&, std::vector<CoordinateOperation> const& candidates)
			{
				for (auto const& candidate : candidates)
					offered.push_back(candidate.Code);

				return chosen;
			};

		auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(sourceCode);
		auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(egm2008Height);

		try
		{
			GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target, options);
		}
		catch (GridFileNotFoundException const&)
		{
		}

		std::ranges::sort(offered);
		return offered;
	}

	// Both EGM2008 operations, sorted: the 2.5' grid and the 1' one.
	std::vector<int> const bothGeoidOperations{ 3858, 3859 };
}

// The catalogued pair: the caller is asked, and both EGM2008 operations are offered.
TEST(OperationSelection, CataloguedGeographicToVerticalPairAsksWhichOperation)
{
	EXPECT_EQ(bothGeoidOperations, CandidatesOfferedFor(wgs84, 3858));
}

// The fallback pair (no catalogued operation from TWD97 to EGM2008 height) must ask just the same.
// It used to take the first geoid operation without asking, silently picking a grid the caller never
// chose -- and leaving the operation dialog of any client without effect.
TEST(OperationSelection, GeoidFallbackAsksWhichOperation)
{
	EXPECT_EQ(bothGeoidOperations, CandidatesOfferedFor(twd97, 3858));
}

// The authority factory is asked for the INVERSE geoid transform when the pair runs vertical ->
// geographic. That request used to be dropped whenever the caller had to choose between several
// operations: the direct transform came back instead, describing itself the wrong way round.
TEST(OperationSelection, TheAuthorityFactoryKeepsTheDirectionWhenTheOperationIsChosen)
{
	CoordinateTransformationOptions options;
	options.selectOperation = [](std::string const&, std::string const&, std::vector<CoordinateOperation> const&)
		{
			return 3858;   // the 2.5' grid
		};

	auto const vertical = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(egm2008Height);
	auto const geographic = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(wgs84);

	std::shared_ptr<ICoordinateTransformation> transformation;
	try
	{
		transformation = GetCoordinateTransformationAuthorityFactory()->CreateFromCoordinateSystems(vertical, geographic, options);
	}
	catch (GridFileNotFoundException const& e)
	{
		GTEST_SKIP() << e.what();
	}

	EXPECT_EQ(egm2008Height, transformation->GetSourceCS()->GetAuthorityCode());
	EXPECT_EQ(wgs84, transformation->GetTargetCS()->GetAuthorityCode());
}

// And with nobody to ask, it must refuse rather than guess -- as the catalogued path already did.
TEST(OperationSelection, GeoidFallbackRefusesToGuessWithoutADelegate)
{
	auto const source = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(twd97);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(egm2008Height);

	EXPECT_THROW(
		GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target),
		TransformationNotFoundException);
}
