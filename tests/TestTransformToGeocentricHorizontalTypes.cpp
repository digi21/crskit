// Utilities::CreateTransformToGeocentric with a horizontal coordinate reference system (CRS) that is neither geographic nor
// projected. Utilities::ExtractHorizontalVertical returns a geocentric CRS as the horizontal CRS and as the vertical CRS.

#include "TestBase.h"
#include "../crskit/IUtilities.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	struct GeocentricChains
	{
		std::vector<std::shared_ptr<IMathTransform>> directTransforms;
		std::vector<std::shared_ptr<IMathTransform>> inverseTransforms;
	};

	auto CreateChainsToGeocentric(std::shared_ptr<CoordinateSystem> const& horizontal, std::shared_ptr<CoordinateSystem> const& vertical) -> GeocentricChains
	{
		GeocentricChains chains;
		GetUtilities()->CreateTransformToGeocentric(horizontal, vertical, chains.directTransforms, chains.inverseTransforms,
			[](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { return nullptr; },
			[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });
		return chains;
	}

	auto ApplyChain(std::vector<std::shared_ptr<IMathTransform>> const& transforms, std::vector<double> point) -> std::vector<double>
	{
		for (auto const& step : transforms)
			point = step->Transform(point);
		return point;
	}

	auto ExpectPointCoordinatesWithinTolerance(std::vector<double> const& expectedPoint, std::vector<double> const& point, double tolerance) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerance) << coordinateIndex;
	}
}

// World Geodetic System 1984 (WGS 84) geocentric (European Petroleum Survey Group (EPSG) code 4978), split by
// ExtractHorizontalVertical: the direct chain and the inverse chain each have one step and return X, Y and Z unchanged.
TEST(TransformToGeocentricHorizontalTypes, GeocentricCoordinateSystemInMetresGivesIdentityChains)
{
	auto const [horizontal, vertical] = GetUtilities()->ExtractHorizontalVertical(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4978));
	auto const chains = CreateChainsToGeocentric(horizontal, vertical);
	ASSERT_EQ(1u, chains.directTransforms.size());
	ASSERT_EQ(1u, chains.inverseTransforms.size());

	std::vector<double> const geocentricPoint{ 4'738'000.0, -305'000.0, 4'220'000.0 };
	ExpectPointCoordinatesWithinTolerance(geocentricPoint, ApplyChain(chains.directTransforms, geocentricPoint), 1e-9);
	ExpectPointCoordinatesWithinTolerance(geocentricPoint, ApplyChain(chains.inverseTransforms, geocentricPoint), 1e-9);
}

// A WGS 84 geocentric CRS in kilometres, defined in Well-Known Text version 2 (WKT2): the direct chain returns X, Y and Z
// in metres, and the inverse chain returns them in kilometres.
TEST(TransformToGeocentricHorizontalTypes, GeocentricCoordinateSystemInKilometresConvertsToMetresAndBack)
{
	auto const geocentricInKilometres = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEODCRS["WGS 84 (km)",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[Cartesian,3],AXIS["(X)",geocentricX],AXIS["(Y)",geocentricY],AXIS["(Z)",geocentricZ],LENGTHUNIT["kilometre",1000]])WKT");
	auto const [horizontal, vertical] = GetUtilities()->ExtractHorizontalVertical(geocentricInKilometres);
	auto const chains = CreateChainsToGeocentric(horizontal, vertical);

	std::vector<double> const pointInKilometres{ 4'738.0, -305.0, 4'220.0 };
	std::vector<double> const pointInMetres{ 4'738'000.0, -305'000.0, 4'220'000.0 };
	ExpectPointCoordinatesWithinTolerance(pointInMetres, ApplyChain(chains.directTransforms, pointInKilometres), 1e-6);
	ExpectPointCoordinatesWithinTolerance(pointInKilometres, ApplyChain(chains.inverseTransforms, pointInMetres), 1e-9);
}

// CreateTransformToGeocentric throws TransformationNotFoundException when horizontal is nullptr or contains the local CRS
// created from the text returned by GetUnknownHorizontalWkt.
TEST(TransformToGeocentricHorizontalTypes, LocalOrNullHorizontalCoordinateSystemThrowsTransformationNotFoundException)
{
	auto const unknownVertical = GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetUnknownVerticalWkt());
	EXPECT_THROW(CreateChainsToGeocentric(GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetUnknownHorizontalWkt()), unknownVertical), TransformationNotFoundException);
	EXPECT_THROW(CreateChainsToGeocentric(nullptr, unknownVertical), TransformationNotFoundException);
}
