// Well-Known Text version 2 (WKT2) coordinate systems that declare the unit inside each AXIS node, as PROJ writes them
// and as WKT2 requires when the axes use different units. The coordinate reference systems (CRSs) have no ID node, so the
// parser builds them from the text.

#include "TestBase.h"

#include <numbers>
#include <string>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	auto CreateCoordinateSystemFromWkt(std::string const& wkt) { return GetCoordinateSystemFactory()->CreateFromWkt(wkt); }
	auto CreateCatalogueCoordinateSystem(int code) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code); }

	constexpr auto kRadiansPerGrad = std::numbers::pi / 200.0;

	auto const kWgs84DatumAndPrimeMeridian = std::string{ R"WKT(DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]])WKT" };
}

// Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian, as PROJ writes it, with latitude and longitude in
// grads: the transformation to NTF (Paris) of the catalogue (European Petroleum Survey Group (EPSG) code 4807), whose
// axes are in grads too, returns the coordinates unchanged.
TEST(Wkt2AxisUnits, GeographicInGradsDeclaredInEachAxis)
{
	auto const geographic = CreateCoordinateSystemFromWkt(
		R"WKT(GEOGCRS["NTF (Paris)",DATUM["Nouvelle Triangulation Francaise (Paris)",ELLIPSOID["Clarke 1880 (IGN)",6378249.2,293.466021293627,LENGTHUNIT["metre",1]]],PRIMEM["Paris",2.5969213,ANGLEUNIT["grad",0.0157079632679489]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["grad",0.0157079632679489]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["grad",0.0157079632679489]]])WKT");
	EXPECT_NEAR(kRadiansPerGrad, std::dynamic_pointer_cast<GeographicCoordinateSystem>(geographic)->GetAngularUnit().GetRadiansPerUnit(), 1e-15);

	auto const transformedPoint = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geographic, CreateCatalogueCoordinateSystem(4807))
		->GetMathTransform()->Transform({ 52.0, 0.5 });
	EXPECT_NEAR(52.0, transformedPoint[0], 1e-12);
	EXPECT_NEAR(0.5, transformedPoint[1], 1e-12);
}

// North American Vertical Datum 1988 (NAVD88) height in US survey feet, as PROJ writes it: the transformation to NAVD88
// height in metres (EPSG code 5703) converts 100 US survey feet to 30.48006096 m.
TEST(Wkt2AxisUnits, VerticalInUsSurveyFeetDeclaredInTheAxis)
{
	auto const vertical = CreateCoordinateSystemFromWkt(
		R"WKT(VERTCRS["NAVD88 height (ftUS)",VDATUM["North American Vertical Datum 1988"],CS[vertical,1],AXIS["gravity-related height (H)",up,LENGTHUNIT["US survey foot",0.304800609601219]]])WKT");
	auto const transformedPoint = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(vertical, CreateCatalogueCoordinateSystem(5703))
		->GetMathTransform()->Transform({ 100.0 });
	EXPECT_NEAR(30.4800609601219, transformedPoint[0], 1e-9);
}

// A geographic 3D CRS with latitude and longitude in grads and the ellipsoidal height in feet.
TEST(Wkt2AxisUnits, Geographic3DWithAHeightUnitDeclaredInTheThirdAxis)
{
	auto const geographic = std::dynamic_pointer_cast<GeographicCoordinateSystem>(CreateCoordinateSystemFromWkt(
		R"WKT(GEOGCRS["WGS 84 in grads and feet",)WKT" + kWgs84DatumAndPrimeMeridian + R"WKT(,CS[ellipsoidal,3],AXIS["geodetic latitude (Lat)",north,ANGLEUNIT["grad",0.0157079632679489]],AXIS["geodetic longitude (Lon)",east,ANGLEUNIT["grad",0.0157079632679489]],AXIS["ellipsoidal height (h)",up,LENGTHUNIT["foot",0.3048]]])WKT"));
	ASSERT_NE(nullptr, geographic);
	EXPECT_NEAR(kRadiansPerGrad, geographic->GetAngularUnit().GetRadiansPerUnit(), 1e-15);
	EXPECT_DOUBLE_EQ(0.3048, std::get<LinearUnit>(geographic->GetUnits(2)).GetMetersPerUnit());
}

// A geocentric CRS with X, Y and Z in kilometres.
TEST(Wkt2AxisUnits, GeocentricInKilometresDeclaredInEachAxis)
{
	auto const geocentric = std::dynamic_pointer_cast<GeocentricCoordinateSystem>(CreateCoordinateSystemFromWkt(
		R"WKT(GEODCRS["WGS 84 geocentric in kilometres",)WKT" + kWgs84DatumAndPrimeMeridian + R"WKT(,CS[Cartesian,3],AXIS["(X)",geocentricX,LENGTHUNIT["kilometre",1000]],AXIS["(Y)",geocentricY,LENGTHUNIT["kilometre",1000]],AXIS["(Z)",geocentricZ,LENGTHUNIT["kilometre",1000]]])WKT"));
	ASSERT_NE(nullptr, geocentric);
	EXPECT_DOUBLE_EQ(1000.0, geocentric->GetLinearUnit().GetMetersPerUnit());
}
