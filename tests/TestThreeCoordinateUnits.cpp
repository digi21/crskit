// Three coordinates of geographic coordinate reference systems (CRSs) with units other than the degree and the metre.
// ConvertsGeographicCoordinatesInGradsToGeocentric and
// ConvertsGeographicCoordinatesWithEllipsoidalHeightInFeetToGeocentric convert coordinates from a World Geodetic System
// 1984 (WGS 84) 3D CRS defined in Well-Known Text version 2 (WKT2) to WGS 84 geocentric (European Petroleum Survey
// Group (EPSG) code 4978) and compare the result with the conversion from WGS 84 3D (EPSG code 4979), whose axes are in
// degrees and metres.

#include "TestBase.h"
#include "../crskit/IUtilities.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

#include <format>
#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	auto const kWgs84DatumAndPrimeMeridianWkt = std::string{ R"WKT(DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]])WKT" };

	// WGS 84 3D with latitude and longitude in angleUnit and the ellipsoidal height in heightUnit.
	auto CreateWgs84Geographic3DCoordinateSystem(std::string const& angleUnit, std::string const& heightUnit) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemFactory()->CreateFromWkt(std::format(
			R"WKT(GEOGCRS["WGS 84 in other units",{},CS[ellipsoidal,3],AXIS["geodetic latitude (Lat)",north,{}],AXIS["geodetic longitude (Lon)",east,{}],AXIS["ellipsoidal height (h)",up,{}]])WKT",
			kWgs84DatumAndPrimeMeridianWkt, angleUnit, angleUnit, heightUnit));
	}

	auto Transform(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, std::vector<double> const& point) -> std::vector<double>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)->GetMathTransform()->Transform(point);
	}

	auto CreateCatalogueCoordinateSystem(int code) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code); }

	// Latitude, longitude (degrees) and ellipsoidal height (metres) in Madrid.
	constexpr double kLatitudeInDegrees = 40.4;
	constexpr double kLongitudeInDegrees = -3.7;
	constexpr double kEllipsoidalHeightInMetres = 700.0;
	constexpr double kGradsPerDegree = 400.0 / 360.0;
	constexpr double kMetresPerFoot = 0.3048;

	auto CalculateExpectedGeocentricCoordinates() -> std::vector<double>
	{
		return Transform(CreateCatalogueCoordinateSystem(4979), CreateCatalogueCoordinateSystem(4978), { kLatitudeInDegrees, kLongitudeInDegrees, kEllipsoidalHeightInMetres });
	}
}

TEST(ThreeCoordinateUnits, ConvertsGeographicCoordinatesInGradsToGeocentric)
{
	auto const geocentricPoint = Transform(CreateWgs84Geographic3DCoordinateSystem(R"WKT(ANGLEUNIT["grad",0.015707963267949])WKT", R"WKT(LENGTHUNIT["metre",1])WKT"),
		CreateCatalogueCoordinateSystem(4978), { kLatitudeInDegrees * kGradsPerDegree, kLongitudeInDegrees * kGradsPerDegree, kEllipsoidalHeightInMetres });
	auto const expectedPoint = CalculateExpectedGeocentricCoordinates();
	for (std::size_t coordinateIndex = 0; coordinateIndex < 3; ++coordinateIndex)
		EXPECT_NEAR(expectedPoint[coordinateIndex], geocentricPoint[coordinateIndex], 1e-3) << coordinateIndex;
}

TEST(ThreeCoordinateUnits, ConvertsGeographicCoordinatesWithEllipsoidalHeightInFeetToGeocentric)
{
	auto const geocentricPoint = Transform(CreateWgs84Geographic3DCoordinateSystem(R"WKT(ANGLEUNIT["degree",0.0174532925199433])WKT", R"WKT(LENGTHUNIT["foot",0.3048])WKT"),
		CreateCatalogueCoordinateSystem(4978), { kLatitudeInDegrees, kLongitudeInDegrees, kEllipsoidalHeightInMetres / kMetresPerFoot });
	auto const expectedPoint = CalculateExpectedGeocentricCoordinates();
	for (std::size_t coordinateIndex = 0; coordinateIndex < 3; ++coordinateIndex)
		EXPECT_NEAR(expectedPoint[coordinateIndex], geocentricPoint[coordinateIndex], 1e-3) << coordinateIndex;
}

// Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian (EPSG code 4807) has latitude and longitude in
// grads. For latitude 52 grads, longitude 0.5 grads and ellipsoidal height 100 m, the direct transformations that
// Utilities::CreateTransformToGeocentric returns, followed by the inverse transformations, return a latitude and a
// longitude that differ from the starting ones by at most 1e-8 grads and an ellipsoidal height that differs from the
// starting one by at most 1 mm.
TEST(ThreeCoordinateUnits, DirectAndInverseGeocentricTransformsRecoverEpsg4807Coordinates)
{
	std::vector<std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform>> directTransformations;
	std::vector<std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform>> inverseTransformations;
	GetUtilities()->CreateTransformToGeocentric(CreateCatalogueCoordinateSystem(4807),
		GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetUnknownVerticalWkt()), directTransformations, inverseTransformations,
		[](std::string const&, std::string const&) -> std::shared_ptr<CrsKit::CoordinateTransformations::IMathTransform> { return nullptr; },
		[](std::string const&, std::string const&, std::vector<CrsKit::CoordinateTransformations::CoordinateOperation> const& operations) { return operations.front().Code; });

	std::vector<double> const startingPoint{ 52.0, 0.5, 100.0 };
	auto point = startingPoint;
	for (auto const& step : directTransformations)
		point = step->Transform(point);
	for (auto const& step : inverseTransformations)
		point = step->Transform(point);
	EXPECT_NEAR(startingPoint[0], point[0], 1e-8);
	EXPECT_NEAR(startingPoint[1], point[1], 1e-8);
	EXPECT_NEAR(startingPoint[2], point[2], 1e-3);
}