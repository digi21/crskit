// HorizontalDatum::CompareTo compares the name of a datum without authority code with the name of another datum after
// converting both names to lower case, replacing each underscore by a space and removing the suffix " ensemble" of the
// datum ensembles of the European Petroleum Survey Group (EPSG) catalogue. The first two tests transform latitude 40
// degrees and longitude -3 degrees from European Terrestrial Reference System 1989 (ETRS89) defined in Well-Known Text
// (WKT) without authority code to ETRS89 / UTM zone 30N (Universal Transverse Mercator; EPSG code 25830), whose datum
// the catalogue names European Terrestrial Reference System 1989 ensemble.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	// Latitude and longitude in degrees.
	std::vector<double> const kEtrs89Point{ 40.0, -3.0 };
	// Easting and northing in metres, from EPSG 4258 to EPSG 25830.
	std::vector<double> const kEtrs89UtmZone30NPoint{ 500000.000000, 4427757.218624 };

	auto CreateMathTransformToEtrs89UtmZone30N(char const* sourceWkt) -> std::shared_ptr<CoordinateTransformations::IMathTransform>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(GetCoordinateSystemFactory()->CreateFromWkt(sourceWkt), GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(25830))->GetMathTransform();
	}

	auto ExpectPointCoordinatesWithinTolerance(std::vector<double> const& expectedPoint, std::vector<double> const& point, double tolerance) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerance) << coordinateIndex;
	}
}

// ETRS89 3D defined in WKT version 2 with the datum name European Terrestrial Reference System 1989.
TEST(DatumNameNormalization, DatumNameWithoutEnsembleSuffixMatchesCatalogueDatumEnsemble)
{
	auto const mathTransform = CreateMathTransformToEtrs89UtmZone30N(
		R"WKT(GEOGCRS["ETRS89 3D",DATUM["European Terrestrial Reference System 1989",ELLIPSOID["GRS 1980",6378137,298.257222101,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,3],AXIS["geodetic latitude (Lat)",north,ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ANGLEUNIT["degree",0.0174532925199433]],AXIS["ellipsoidal height (h)",up,LENGTHUNIT["metre",1]]])WKT");
	ExpectPointCoordinatesWithinTolerance(kEtrs89UtmZone30NPoint, mathTransform->Transform(kEtrs89Point), 1e-3);
}

// ETRS89 2D defined in WKT version 1 with the datum name European_Terrestrial_Reference_System_1989.
TEST(DatumNameNormalization, DatumNameWithUnderscoresMatchesCatalogueDatumEnsemble)
{
	auto const mathTransform = CreateMathTransformToEtrs89UtmZone30N(
		R"WKT(GEOGCS["ETRS89 2D",DATUM["European_Terrestrial_Reference_System_1989",SPHEROID["GRS 1980",6378137,298.257222101]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT");
	ExpectPointCoordinatesWithinTolerance(kEtrs89UtmZone30NPoint, mathTransform->Transform(kEtrs89Point), 1e-3);
}

// CreateFromCoordinateSystems throws TransformationNotFoundException when asked to create a transformation from a
// geographic 2D coordinate reference system defined in WKT version 1 without authority code, without TOWGS84 parameters
// for the transformation to World Geodetic System 1984, with the Geodetic Reference System 1980 ellipsoid and with the
// datum name Test_Reference_System_1989 to EPSG 25830.
TEST(DatumNameNormalization, DifferentDatumNameThrowsTransformationNotFoundException)
{
	EXPECT_THROW(static_cast<void>(CreateMathTransformToEtrs89UtmZone30N(
		R"WKT(GEOGCS["Test 2D",DATUM["Test_Reference_System_1989",SPHEROID["GRS 1980",6378137,298.257222101]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT")), TransformationNotFoundException);
}
