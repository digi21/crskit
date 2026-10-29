// Geocentric coordinates and datum shifts through geocentric coordinates from a geographic coordinate reference system
// (CRS) whose prime meridian is not Greenwich. The geocentric conversion receives longitudes from Greenwich, so the same
// point in a CRS with the Paris prime meridian and in a CRS with the Greenwich prime meridian of the same datum gives the
// same result. The Paris meridian is 2.5969213 grads, 2.33722917 degrees, east of Greenwich.

#include "TestBase.h"
#include "../crskit/IUtilities.h"
#include "../crskit/CoordinateTransformations/CoordinateOperation.h"

#include <cmath>
#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	constexpr double kParisLongitudeInDegrees = 2.5969213 * 0.9;
	constexpr double kLatitudeInDegrees = 46.8;
	constexpr double kLongitudeFromParisInDegrees = 0.45;
	// Subtracting the longitude of the Paris prime meridian from -179 degrees from Greenwich gives -181.33722917 degrees;
	// reducing that longitude to [-180, 180] degrees gives 178.66277083 degrees.
	constexpr double kLongitudeFromGreenwichNearTheAntimeridianInDegrees = -179.0;

	auto CreateCatalogueCoordinateSystem(int epsgCode) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(epsgCode); }

	// Applies the direct chain (toGeocentric true) or the inverse chain (toGeocentric false) of
	// Utilities::CreateTransformToGeocentric for horizontal and the unknown vertical coordinate reference system.
	auto ApplyGeocentricChain(std::shared_ptr<CoordinateSystem> const& horizontal, std::vector<double> point, bool toGeocentric) -> std::vector<double>
	{
		std::vector<std::shared_ptr<IMathTransform>> directTransformations;
		std::vector<std::shared_ptr<IMathTransform>> inverseTransformations;
		GetUtilities()->CreateTransformToGeocentric(horizontal, GetCoordinateSystemFactory()->CreateFromWkt(GetUtilities()->GetUnknownVerticalWkt()),
			directTransformations, inverseTransformations,
			[](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform> { return nullptr; },
			[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });
		for (auto const& step : toGeocentric ? directTransformations : inverseTransformations)
			point = step->Transform(point);
		return point;
	}

	auto TransformToGeocentric(std::shared_ptr<CoordinateSystem> const& horizontal, std::vector<double> const& point) -> std::vector<double>
	{
		return ApplyGeocentricChain(horizontal, point, true);
	}

	// Returns the longitude from the Paris prime meridian in [-180, 180] degrees for a longitude from Greenwich in degrees.
	auto CalculateLongitudeFromParisInDegrees(double longitudeFromGreenwichInDegrees) -> double
	{
		return std::remainder(longitudeFromGreenwichInDegrees - kParisLongitudeInDegrees, 360.0);
	}

	auto ExpectPointCoordinatesWithinTolerance(std::vector<double> const& expectedPoint, std::vector<double> const& point, double tolerance) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerance) << coordinateIndex;
	}
}

// Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian (European Petroleum Survey Group (EPSG) code 4807,
// grads) and NTF with the Greenwich prime meridian (EPSG code 4275, degrees).
TEST(PrimeMeridianGeocentric, TransformToGeocentricAddsTheLongitudeOfTheParisMeridian)
{
	auto const geocentricPointFromParisCoordinates = TransformToGeocentric(CreateCatalogueCoordinateSystem(4807), { kLatitudeInDegrees / 0.9, kLongitudeFromParisInDegrees / 0.9, 100.0 });
	auto const geocentricPointFromGreenwichCoordinates = TransformToGeocentric(CreateCatalogueCoordinateSystem(4275), { kLatitudeInDegrees, kLongitudeFromParisInDegrees + kParisLongitudeInDegrees, 100.0 });
	ExpectPointCoordinatesWithinTolerance(geocentricPointFromGreenwichCoordinates, geocentricPointFromParisCoordinates, 1e-3);
}

// A three-dimensional World Geodetic System 1984 (WGS 84) CRS with the Paris prime meridian, defined in Well-Known Text
// version 2 (WKT2), to WGS 84 geocentric (EPSG code 4978), compared with WGS 84 3D (EPSG code 4979) to EPSG 4978.
TEST(PrimeMeridianGeocentric, CreateFromCoordinateSystemsAddsParisMeridianLongitudeBeforeGeocentricConversion)
{
	auto const geographicWithParisMeridian = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCRS["WGS 84 (Paris)",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Paris",2.5969213,ANGLEUNIT["grad",0.0157079632679489]],CS[ellipsoidal,3],AXIS["geodetic latitude (Lat)",north,ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ANGLEUNIT["degree",0.0174532925199433]],AXIS["ellipsoidal height (h)",up,LENGTHUNIT["metre",1]]])WKT");
	auto const geocentric = CreateCatalogueCoordinateSystem(4978);
	auto const geocentricPointFromParisCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geographicWithParisMeridian, geocentric)->GetMathTransform()
		->Transform({ kLatitudeInDegrees, kLongitudeFromParisInDegrees, 100.0 });
	auto const geocentricPointFromGreenwichCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCatalogueCoordinateSystem(4979), geocentric)->GetMathTransform()
		->Transform({ kLatitudeInDegrees, kLongitudeFromParisInDegrees + kParisLongitudeInDegrees, 100.0 });
	ExpectPointCoordinatesWithinTolerance(geocentricPointFromGreenwichCoordinates, geocentricPointFromParisCoordinates, 1e-3);
}

// NTF with the Paris prime meridian and NTF with the Greenwich prime meridian, both with the TOWGS84 parameters of the NTF
// datum, as the Geospatial Data Abstraction Library (GDAL) writes the two coordinate reference systems in Well-Known Text
// version 1 (WKT1), with the longitude of PRIMEM in degrees, to WGS 84 (EPSG code 4326): the datum shift through
// geocentric coordinates gives the same point.
TEST(PrimeMeridianGeocentric, Towgs84DatumShiftAddsTheLongitudeOfTheParisMeridian)
{
	auto const ntfParis = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCS["NTF (Paris)",DATUM["Nouvelle_Triangulation_Francaise_Paris",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM["Paris",2.33722917,AUTHORITY["EPSG","8903"]],UNIT["grad",0.01570796326794897],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT");
	auto const ntfGreenwich = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCS["NTF",DATUM["Nouvelle_Triangulation_Francaise",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT");
	auto const wgs84 = CreateCatalogueCoordinateSystem(4326);
	auto const wgs84GeographicPointFromParisCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(ntfParis, wgs84)->GetMathTransform()
		->Transform({ kLatitudeInDegrees / 0.9, kLongitudeFromParisInDegrees / 0.9 });
	auto const wgs84GeographicPointFromGreenwichCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(ntfGreenwich, wgs84)->GetMathTransform()
		->Transform({ kLatitudeInDegrees, kLongitudeFromParisInDegrees + kParisLongitudeInDegrees });
	ExpectPointCoordinatesWithinTolerance(wgs84GeographicPointFromGreenwichCoordinates, wgs84GeographicPointFromParisCoordinates, 1e-8);
}

// The inverse chain of CreateTransformToGeocentric for Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian
// (European Petroleum Survey Group (EPSG) code 4807, grads) returns the longitude from the Paris prime meridian in
// [-200, 200] grads for a geocentric point 179 degrees west of Greenwich.
TEST(PrimeMeridianGeocentric, TransformToGeocentricInverseChainReturnsParisLongitudeBetweenMinus200And200Grads)
{
	auto const geocentricPoint = TransformToGeocentric(CreateCatalogueCoordinateSystem(4275), { kLatitudeInDegrees, kLongitudeFromGreenwichNearTheAntimeridianInDegrees, 100.0 });
	auto const parisPointWithAnglesInGrads = ApplyGeocentricChain(CreateCatalogueCoordinateSystem(4807), geocentricPoint, false);
	ExpectPointCoordinatesWithinTolerance({ kLatitudeInDegrees / 0.9, CalculateLongitudeFromParisInDegrees(kLongitudeFromGreenwichNearTheAntimeridianInDegrees) / 0.9, 100.0 }, parisPointWithAnglesInGrads, 1e-8);
}

// The conversion from World Geodetic System 1984 (WGS 84) geocentric (European Petroleum Survey Group (EPSG) code 4978) to
// a three-dimensional WGS 84 coordinate reference system with the Paris prime meridian, defined in Well-Known Text version
// 2, returns the longitude from the Paris prime meridian in [-180, 180] degrees for a geocentric point 179 degrees west of
// Greenwich.
TEST(PrimeMeridianGeocentric, CreateFromCoordinateSystemsReturnsParisLongitudeBetweenMinus180And180Degrees)
{
	auto const geographicWithParisMeridian = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCRS["WGS 84 (Paris)",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Paris",2.5969213,ANGLEUNIT["grad",0.0157079632679489]],CS[ellipsoidal,3],AXIS["geodetic latitude (Lat)",north,ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ANGLEUNIT["degree",0.0174532925199433]],AXIS["ellipsoidal height (h)",up,LENGTHUNIT["metre",1]]])WKT");
	auto const geocentric = CreateCatalogueCoordinateSystem(4978);
	auto const geocentricPoint = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCatalogueCoordinateSystem(4979), geocentric)->GetMathTransform()
		->Transform({ kLatitudeInDegrees, kLongitudeFromGreenwichNearTheAntimeridianInDegrees, 100.0 });
	auto const geographicPoint = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geocentric, geographicWithParisMeridian)->GetMathTransform()
		->Transform(geocentricPoint);
	ExpectPointCoordinatesWithinTolerance({ kLatitudeInDegrees, CalculateLongitudeFromParisInDegrees(kLongitudeFromGreenwichNearTheAntimeridianInDegrees), 100.0 }, geographicPoint, 1e-8);
}

// The test calls CreateTransformToGeocentric for Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian and
// for NTF with the Greenwich prime meridian, both geographic coordinate reference systems (CRSs) defined in Well-Known Text
// version 1 without authority code and with angles in degrees, and for NGF-IGN69 height (European Petroleum Survey Group
// (EPSG) code 5720). The catalogue has no geographic 3D CRS for these CRSs, so createVerticalSystemDialogCallback returns the
// transformation from the geographic CRS to the vertical CRS: an affine transformation that subtracts the longitude from
// Greenwich in degrees from the ellipsoidal height in metres. The test compares the geocentric coordinates of the same point
// returned by the Paris and Greenwich direct chains with a tolerance of 1e-3 metres per coordinate, and checks that the
// Paris inverse chain returns the starting latitude and longitude within 1e-8 degrees and the starting height within 1e-8
// metres.
TEST(PrimeMeridianGeocentric, TransformToGeocentricPassesLongitudeFromGreenwichToTheTransformationOfTheDialog)
{
	auto const createChains = [](std::string const& horizontalWkt, std::vector<std::shared_ptr<IMathTransform>>& directTransformations, std::vector<std::shared_ptr<IMathTransform>>& inverseTransformations)
	{
		GetUtilities()->CreateTransformToGeocentric(GetCoordinateSystemFactory()->CreateFromWkt(horizontalWkt), CreateCatalogueCoordinateSystem(5720),
			directTransformations, inverseTransformations,
			[](std::string const&, std::string const&) -> std::shared_ptr<IMathTransform>
			{
				return GetMathTransformFactory()->CreateFromWkt(
					R"WKT(PARAM_MT["Affine",PARAMETER["num_row",4],PARAMETER["num_col",4],PARAMETER["elt_0_0",1],PARAMETER["elt_0_1",0],PARAMETER["elt_0_2",0],PARAMETER["elt_0_3",0],PARAMETER["elt_1_0",0],PARAMETER["elt_1_1",1],PARAMETER["elt_1_2",0],PARAMETER["elt_1_3",0],PARAMETER["elt_2_0",-1],PARAMETER["elt_2_1",0],PARAMETER["elt_2_2",1],PARAMETER["elt_2_3",0],PARAMETER["elt_3_0",0],PARAMETER["elt_3_1",0],PARAMETER["elt_3_2",0],PARAMETER["elt_3_3",1]])WKT");
			},
			[](std::string const&, std::string const&, std::vector<CoordinateOperation> const& operations) { return operations.front().Code; });
	};
	auto const applySteps = [](std::vector<std::shared_ptr<IMathTransform>> const& steps, std::vector<double> point)
	{
		for (auto const& step : steps)
			point = step->Transform(point);
		return point;
	};

	std::vector<std::shared_ptr<IMathTransform>> parisDirectTransformations;
	std::vector<std::shared_ptr<IMathTransform>> parisInverseTransformations;
	createChains(R"WKT(GEOGCS["NTF (Paris) in degrees",DATUM["Test_NTF",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269]],PRIMEM["Paris",2.33722917],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT",
		parisDirectTransformations, parisInverseTransformations);
	std::vector<std::shared_ptr<IMathTransform>> greenwichDirectTransformations;
	std::vector<std::shared_ptr<IMathTransform>> greenwichInverseTransformations;
	createChains(R"WKT(GEOGCS["NTF (Greenwich) in degrees",DATUM["Test_NTF",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT",
		greenwichDirectTransformations, greenwichInverseTransformations);

	std::vector<double> const parisPoint{ kLatitudeInDegrees, kLongitudeFromParisInDegrees, 100.0 };
	auto const geocentricPointFromParisCoordinates = applySteps(parisDirectTransformations, parisPoint);
	auto const geocentricPointFromGreenwichCoordinates = applySteps(greenwichDirectTransformations, { kLatitudeInDegrees, kLongitudeFromParisInDegrees + kParisLongitudeInDegrees, 100.0 });
	ExpectPointCoordinatesWithinTolerance(geocentricPointFromGreenwichCoordinates, geocentricPointFromParisCoordinates, 1e-3);
	ExpectPointCoordinatesWithinTolerance(parisPoint, applySteps(parisInverseTransformations, geocentricPointFromParisCoordinates), 1e-8);
}

// A World Geodetic System 1984 (WGS 84) geocentric coordinate reference system with the Paris prime meridian, defined in
// Well-Known Text version 1 without authority code, has its X axis in the plane of the Paris meridian. At the same latitude
// and ellipsoidal height, a point with a given longitude from Paris has in this CRS the coordinate values that a point with
// that longitude from Greenwich has in WGS 84 geocentric (EPSG code 4978). The tests obtain both sets of coordinates with
// the transformation from WGS 84 3D (EPSG code 4979) to EPSG 4978.
namespace
{
	constexpr char const* kWgs84GeocentricWithParisMeridianWkt = R"WKT(GEOCCS["WGS 84 geocentric (Paris)",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Paris",2.33722917],UNIT["metre",1]])WKT";

	// Returns the coordinates in EPSG 4978 of the point at kLatitudeInDegrees, longitudeInDegrees from Greenwich and an
	// ellipsoidal height of 100 metres.
	auto CalculateWgs84GeocentricPoint(double longitudeInDegrees) -> std::vector<double>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCatalogueCoordinateSystem(4979), CreateCatalogueCoordinateSystem(4978))->GetMathTransform()
			->Transform({ kLatitudeInDegrees, longitudeInDegrees, 100.0 });
	}
}

// The transformation from the WGS 84 geocentric CRS with the Paris prime meridian to EPSG 4978 rotates the coordinates by
// the longitude of the Paris meridian about the Z axis, and its inverse rotates EPSG 4978 coordinates by the opposite angle.
TEST(PrimeMeridianGeocentric, GeocentricWithParisPrimeMeridianToWgs84GeocentricRotatesAboutTheZAxis)
{
	auto const parisGeocentricPoint = CalculateWgs84GeocentricPoint(kLongitudeFromParisInDegrees);
	auto const greenwichGeocentricPoint = CalculateWgs84GeocentricPoint(kLongitudeFromParisInDegrees + kParisLongitudeInDegrees);
	auto const mathTransform = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(GetCoordinateSystemFactory()->CreateFromWkt(kWgs84GeocentricWithParisMeridianWkt), CreateCatalogueCoordinateSystem(4978))->GetMathTransform();
	ExpectPointCoordinatesWithinTolerance(greenwichGeocentricPoint, mathTransform->Transform(parisGeocentricPoint), 1e-3);
	ExpectPointCoordinatesWithinTolerance(parisGeocentricPoint, mathTransform->GetInverse()->Transform(greenwichGeocentricPoint), 1e-3);
}

// The transformation from EPSG 4979 to the WGS 84 geocentric CRS with the Paris prime meridian converts the longitude from
// Greenwich to the longitude from Paris before the geocentric conversion.
TEST(PrimeMeridianGeocentric, Wgs84ToGeocentricWithParisPrimeMeridianConvertsLongitudeFromGreenwichToParis)
{
	auto const mathTransform = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(CreateCatalogueCoordinateSystem(4979), GetCoordinateSystemFactory()->CreateFromWkt(kWgs84GeocentricWithParisMeridianWkt))->GetMathTransform();
	ExpectPointCoordinatesWithinTolerance(CalculateWgs84GeocentricPoint(kLongitudeFromParisInDegrees), mathTransform->Transform({ kLatitudeInDegrees, kLongitudeFromParisInDegrees + kParisLongitudeInDegrees, 100.0 }), 1e-3);
}

// The direct chain of CreateTransformToGeocentric for the WGS 84 geocentric CRS with the Paris prime meridian returns the
// coordinates in EPSG 4978, and the inverse chain converts EPSG 4978 coordinates to coordinates in the WGS 84 geocentric
// CRS with the Paris prime meridian.
TEST(PrimeMeridianGeocentric, TransformToGeocentricRotatesGeocentricCoordinatesWithParisPrimeMeridian)
{
	auto const geocentricWithParisMeridian = GetCoordinateSystemFactory()->CreateFromWkt(kWgs84GeocentricWithParisMeridianWkt);
	auto const parisGeocentricPoint = CalculateWgs84GeocentricPoint(kLongitudeFromParisInDegrees);
	auto const greenwichGeocentricPoint = ApplyGeocentricChain(geocentricWithParisMeridian, parisGeocentricPoint, true);
	ExpectPointCoordinatesWithinTolerance(CalculateWgs84GeocentricPoint(kLongitudeFromParisInDegrees + kParisLongitudeInDegrees), greenwichGeocentricPoint, 1e-3);
	ExpectPointCoordinatesWithinTolerance(parisGeocentricPoint, ApplyGeocentricChain(geocentricWithParisMeridian, greenwichGeocentricPoint, false), 1e-3);
}

// The datum shift from World Geodetic System 1984 (WGS 84; European Petroleum Survey Group (EPSG) code 4326) to Nouvelle
// Triangulation Francaise (NTF) with the Paris prime meridian, defined in Well-Known Text version 1 (WKT1) with TOWGS84
// parameters, returns for a point 179 degrees west of Greenwich the longitude from the Paris prime meridian in [-200, 200]
// grads that corresponds to the longitude returned by the datum shift to NTF with the Greenwich prime meridian and the
// same TOWGS84 parameters.
TEST(PrimeMeridianGeocentric, Towgs84DatumShiftReturnsParisLongitudeBetweenMinus200And200Grads)
{
	auto const ntfParis = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCS["NTF (Paris)",DATUM["Nouvelle_Triangulation_Francaise_Paris",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM["Paris",2.33722917,AUTHORITY["EPSG","8903"]],UNIT["grad",0.01570796326794897],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT");
	auto const ntfGreenwich = GetCoordinateSystemFactory()->CreateFromWkt(
		R"WKT(GEOGCS["NTF",DATUM["Nouvelle_Triangulation_Francaise",SPHEROID["Clarke 1880 (IGN)",6378249.2,293.4660212936269],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT");
	auto const wgs84 = CreateCatalogueCoordinateSystem(4326);
	std::vector<double> const wgs84Point{ kLatitudeInDegrees, kLongitudeFromGreenwichNearTheAntimeridianInDegrees };
	auto const ntfPointInGrads = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(wgs84, ntfParis)->GetMathTransform()->Transform(wgs84Point);
	auto const ntfPointInDegrees = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(wgs84, ntfGreenwich)->GetMathTransform()->Transform(wgs84Point);
	ExpectPointCoordinatesWithinTolerance({ ntfPointInDegrees[0] / 0.9, CalculateLongitudeFromParisInDegrees(ntfPointInDegrees[1]) / 0.9 }, ntfPointInGrads, 1e-8);
}