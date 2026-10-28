// The tests check that HasLatitudeFirst reads the axis orientations, and that the factory transforms the coordinates of a
// geographic coordinate reference system written by PROJ in Well-Known Text version 2 (WKT2), whose axes are named
// "geodetic latitude (Lat)" and "geodetic longitude (Lon)", in the axis order that the definition declares.

#include "TestBase.h"
#include "../crskit/CoordinateTransformations/CoordinateTransformationFactory.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;

namespace
{
	// European Terrestrial Reference System 1989 (ETRS89, EPSG:4258) as PROJ writes it in WKT2; the two definitions use the
	// latitude-longitude and the longitude-latitude axis orders.
	char const* kWkt2Etrs89LatitudeFirst = R"WKT(GEOGCRS["ETRS89",ENSEMBLE["European Terrestrial Reference System 1989 ensemble",MEMBER["European Terrestrial Reference Frame 1989"],ELLIPSOID["GRS 1980",6378137,298.257222101,LENGTHUNIT["metre",1]],ENSEMBLEACCURACY[0.1],ID["EPSG",6258]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4258]])WKT";
	char const* kWkt2Etrs89LongitudeFirst = R"WKT(GEOGCRS["ETRS89",ENSEMBLE["European Terrestrial Reference System 1989 ensemble",MEMBER["European Terrestrial Reference Frame 1989"],ELLIPSOID["GRS 1980",6378137,298.257222101,LENGTHUNIT["metre",1]],ENSEMBLEACCURACY[0.1],ID["EPSG",6258]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic longitude (Lon)",east,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic latitude (Lat)",north,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4258]])WKT";

	// Projects latitude 40 degrees and longitude -3 degrees from the catalogue definition of EPSG:4258 to the catalogue
	// definition of EPSG:25830.
	auto ProjectTestPointFromEpsg4258ToEpsg25830() -> std::vector<double>
	{
		auto const catalogueGeographic = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4258);
		auto const catalogueProjected = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(25830);
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(catalogueGeographic, catalogueProjected)->GetMathTransform()->Transform({ 40.0, -3.0 });
	}
}

TEST(GeographicAxisOrder, HasLatitudeFirstReadsTheOrientations)
{
	EXPECT_TRUE(StaticCoordinateTransformationFactory::HasLatitudeFirst(*GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326)));
	EXPECT_TRUE(StaticCoordinateTransformationFactory::HasLatitudeFirst(*GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Etrs89LatitudeFirst)));
	EXPECT_FALSE(StaticCoordinateTransformationFactory::HasLatitudeFirst(*GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Etrs89LongitudeFirst)));
}

TEST(GeographicAxisOrder, Wkt2LatitudeFirstMatchesEpsg4258ToEpsg25830ProjectedCoordinates)
{
	auto const source = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Etrs89LatitudeFirst);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(25830);
	auto const expectedCoordinates = ProjectTestPointFromEpsg4258ToEpsg25830();

	auto const actualCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)->GetMathTransform()->Transform({ 40.0, -3.0 });
	EXPECT_NEAR(expectedCoordinates[0], actualCoordinates[0], 1e-6);
	EXPECT_NEAR(expectedCoordinates[1], actualCoordinates[1], 1e-6);
}

TEST(GeographicAxisOrder, Wkt2LongitudeFirstMatchesEpsg4258ToEpsg25830ProjectedCoordinates)
{
	auto const source = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Etrs89LongitudeFirst);
	auto const target = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(25830);
	auto const expectedCoordinates = ProjectTestPointFromEpsg4258ToEpsg25830();

	auto const actualCoordinates = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target)->GetMathTransform()->Transform({ -3.0, 40.0 });
	EXPECT_NEAR(expectedCoordinates[0], actualCoordinates[0], 1e-6);
	EXPECT_NEAR(expectedCoordinates[1], actualCoordinates[1], 1e-6);
}