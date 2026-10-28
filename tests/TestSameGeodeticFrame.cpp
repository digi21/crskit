// The factory transforms coordinates between a coordinate reference system (CRS) without an authority code and a catalogue
// CRS on the same geodetic frame (same datum, ellipsoid and prime meridian): it removes the projection of the source if the
// source is projected, converts the axes and the angular unit, and applies the projection of the target if the target is
// projected.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	// European Terrestrial Reference System 1989 (ETRS89) in Well-Known Text version 1 (WKT1), without the AUTHORITY element
	// of the CRS, latitude first.
	char const* kWkt1Etrs89WithoutCode = R"WKT(GEOGCS["ETRS89",DATUM["European_Terrestrial_Reference_System_1989",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6258"]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat",NORTH],AXIS["Long",EAST]])WKT";

	// ETRS89 as PROJ writes it in Well-Known Text version 2 (WKT2), without the ID element of the CRS, longitude first.
	char const* kWkt2Etrs89WithoutCodeLongitudeFirst = R"WKT(GEOGCRS["ETRS89",ENSEMBLE["European Terrestrial Reference System 1989 ensemble",MEMBER["European Terrestrial Reference Frame 1989"],ELLIPSOID["GRS 1980",6378137,298.257222101,LENGTHUNIT["metre",1]],ENSEMBLEACCURACY[0.1],ID["EPSG",6258]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic longitude (Lon)",east,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic latitude (Lat)",north,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]]])WKT";

	// ETRS89 in WKT1 without code, in grads, latitude first.
	char const* kWkt1Etrs89InGradsWithoutCode = R"WKT(GEOGCS["ETRS89 in grads",DATUM["European_Terrestrial_Reference_System_1989",SPHEROID["GRS 1980",6378137,298.257222101],AUTHORITY["EPSG","6258"]],PRIMEM["Greenwich",0],UNIT["grad",0.01570796326794897],AXIS["Lat",NORTH],AXIS["Long",EAST]])WKT";

	// ETRS89 / Universal Transverse Mercator (UTM) zone 31N in WKT1, without the AUTHORITY element of the CRS.
	char const* kWkt1Etrs89Utm31nWithoutCode = R"WKT(PROJCS["ETRS89 / UTM zone 31N",GEOGCS["ETRS89",DATUM["European_Terrestrial_Reference_System_1989",SPHEROID["GRS 1980",6378137,298.257222101],AUTHORITY["EPSG","6258"]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Lat",NORTH],AXIS["Long",EAST]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",3],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],UNIT["metre",1],AXIS["Easting",EAST],AXIS["Northing",NORTH]])WKT";

	auto CreateCatalogueCoordinateSystem(int code) { return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code); }

	auto Transform(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, std::vector<double> const& coordinates)
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)->GetMathTransform()->Transform(coordinates);
	}
}

TEST(SameGeodeticFrame, GeographicWithoutCodeProjectsLikeTheCatalogue)
{
	auto const expectedCoordinates = Transform(CreateCatalogueCoordinateSystem(4258), CreateCatalogueCoordinateSystem(25830), { 40.0, -3.0 });
	auto const actualCoordinates = Transform(GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Etrs89WithoutCode), CreateCatalogueCoordinateSystem(25830), { 40.0, -3.0 });
	EXPECT_NEAR(expectedCoordinates[0], actualCoordinates[0], 1e-6);
	EXPECT_NEAR(expectedCoordinates[1], actualCoordinates[1], 1e-6);
}

TEST(SameGeodeticFrame, Wkt2WithoutCodeAndLongitudeFirstProjectsLikeTheCatalogue)
{
	auto const expectedCoordinates = Transform(CreateCatalogueCoordinateSystem(4258), CreateCatalogueCoordinateSystem(25830), { 40.0, -3.0 });
	auto const actualCoordinates = Transform(GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Etrs89WithoutCodeLongitudeFirst), CreateCatalogueCoordinateSystem(25830), { -3.0, 40.0 });
	EXPECT_NEAR(expectedCoordinates[0], actualCoordinates[0], 1e-6);
	EXPECT_NEAR(expectedCoordinates[1], actualCoordinates[1], 1e-6);
}

TEST(SameGeodeticFrame, CatalogueProjectedUnprojectsToGeographicWithoutCode)
{
	auto const projectedCoordinates = Transform(CreateCatalogueCoordinateSystem(4258), CreateCatalogueCoordinateSystem(25830), { 40.0, -3.0 });
	auto const actualCoordinates = Transform(CreateCatalogueCoordinateSystem(25830), GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Etrs89WithoutCode), projectedCoordinates);
	EXPECT_NEAR(40.0, actualCoordinates[0], 1e-9);
	EXPECT_NEAR(-3.0, actualCoordinates[1], 1e-9);
}

TEST(SameGeodeticFrame, GeographicWithoutCodeInGradsConvertsToTheCatalogueInDegrees)
{
	// 40 degrees are 44.4... grads and -3 degrees are -3.33... grads.
	auto const actualCoordinates = Transform(GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Etrs89InGradsWithoutCode), CreateCatalogueCoordinateSystem(4258), { 400.0 / 9.0, -10.0 / 3.0 });
	EXPECT_NEAR(40.0, actualCoordinates[0], 1e-9);
	EXPECT_NEAR(-3.0, actualCoordinates[1], 1e-9);
}

TEST(SameGeodeticFrame, CatalogueProjectedReprojectsToProjectedWithoutCode)
{
	auto const projectedCoordinates = Transform(CreateCatalogueCoordinateSystem(4258), CreateCatalogueCoordinateSystem(25830), { 40.0, -3.0 });
	auto const expectedCoordinates = Transform(CreateCatalogueCoordinateSystem(25830), CreateCatalogueCoordinateSystem(25831), projectedCoordinates);
	auto const actualCoordinates = Transform(CreateCatalogueCoordinateSystem(25830), GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Etrs89Utm31nWithoutCode), projectedCoordinates);
	EXPECT_NEAR(expectedCoordinates[0], actualCoordinates[0], 1e-6);
	EXPECT_NEAR(expectedCoordinates[1], actualCoordinates[1], 1e-6);
}