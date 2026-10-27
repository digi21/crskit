// WKT2 (ISO 19162:2019) emission and parsing. Not part of the GIGS conformance battery, so the suite
// name avoids the "Test5xxx" prefix.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	// Restores CrsContext::wktVersion on scope exit so a test selecting WKT2 emission does not
	// leak into the rest of the suite (the default context is process-global).
	struct ScopedWktVersion
	{
		WktVersion previous;
		explicit ScopedWktVersion(WktVersion version)
			: previous{GetDefaultContext()->wktVersion}
		{
			GetDefaultContext()->wktVersion = version;
		}
		~ScopedWktVersion() { GetDefaultContext()->wktVersion = previous; }
	};

	// A user-defined WKT1 geographic CRS (unknown name, no AUTHORITY) so it is built from its parts rather
	// than snapped to an EPSG catalogue entry.
	const char* kWkt1Geographic =
		R"WKT(GEOGCS["Test GCS",DATUM["Test Datum",SPHEROID["Test ellipsoid",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]])WKT";

	// A WKT2:2019 geographic CRS as emitted by PROJ/GDAL (whitespace, no ORDER, cs-level ANGLEUNIT). A
	// custom raw-string delimiter is required because the axis names contain ")"" which would otherwise
	// close a plain R"(...)" literal.
	const char* kWkt2Wgs84 =
		R"WKT(GEOGCRS["WGS 84",
    DATUM["World Geodetic System 1984",
        ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],
    PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],
    CS[ellipsoidal,2],
        AXIS["geodetic latitude (Lat)",north],
        AXIS["geodetic longitude (Lon)",east],
        ANGLEUNIT["degree",0.0174532925199433],
    ID["EPSG",4326]])WKT";

	// A user-defined WKT1 projected CRS (Transverse Mercator, unknown name) built from its parts.
	const char* kWkt1Projected =
		R"WKT(PROJCS["Test PCS",GEOGCS["Test GCS",DATUM["Test Datum",SPHEROID["Test ellipsoid",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",3],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],UNIT["metre",1]])WKT";

	// A WKT2:2019 projected CRS as emitted by PROJ/GDAL (WGS 84 / UTM zone 31N).
	const char* kWkt2Utm31N =
		R"WKT(PROJCRS["WGS 84 / UTM zone 31N",
    BASEGEOGCRS["WGS 84",
        DATUM["World Geodetic System 1984",
            ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],
        PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]]],
    CONVERSION["UTM zone 31N",
        METHOD["Transverse Mercator",ID["EPSG",9807]],
        PARAMETER["Latitude of natural origin",0,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8801]],
        PARAMETER["Longitude of natural origin",3,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8802]],
        PARAMETER["Scale factor at natural origin",0.9996,SCALEUNIT["unity",1],ID["EPSG",8805]],
        PARAMETER["False easting",500000,LENGTHUNIT["metre",1],ID["EPSG",8806]],
        PARAMETER["False northing",0,LENGTHUNIT["metre",1],ID["EPSG",8807]]],
    CS[Cartesian,2],
        AXIS["(E)",east],
        AXIS["(N)",north],
        LENGTHUNIT["metre",1],
    ID["EPSG",32631]])WKT";

	// A WKT2:2019 geocentric CRS (WGS 84, EPSG:4978).
	const char* kWkt2Geocentric =
		R"WKT(GEODCRS["WGS 84",
    DATUM["World Geodetic System 1984",
        ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],
    PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],
    CS[Cartesian,3],
        AXIS["(X)",geocentricX],
        AXIS["(Y)",geocentricY],
        AXIS["(Z)",geocentricZ],
        LENGTHUNIT["metre",1],
    ID["EPSG",4978]])WKT";

	// A WKT2:2019 vertical CRS (EGM2008 height, EPSG:3855).
	const char* kWkt2Vertical =
		R"WKT(VERTCRS["EGM2008 height",
    VDATUM["EGM2008 geoid"],
    CS[vertical,1],
        AXIS["gravity-related height (H)",up],
        LENGTHUNIT["metre",1],
    ID["EPSG",3855]])WKT";

	// A projected coordinate reference system (CRS), encoded as Well-known text version 2 (WKT2:2019), from a compressed
	// point-cloud file (.laz) published by Spain's Instituto Geográfico Nacional (IGN) for the 2023 Plan Nacional de
	// Ortofotografía Aérea (PNOA). The projected CRS uses a datum ensemble in its base geographic CRS; the ensemble's
	// ellipsoid has an inverse flattening rounded to 298.2572221.
	const char* kWkt2Etrs89Utm30NEnsemble =
		R"WKT(PROJCRS["ETRS89 / UTM zone 30N",BASEGEOGCRS["ETRS89",ENSEMBLE["European Terrestrial Reference System 1989 ensemble", MEMBER["European Terrestrial Reference Frame 1989", ID["EPSG",1178]], MEMBER["European Terrestrial Reference Frame 1990", ID["EPSG",1179]], MEMBER["European Terrestrial Reference Frame 1991", ID["EPSG",1180]], MEMBER["European Terrestrial Reference Frame 1992", ID["EPSG",1181]], MEMBER["European Terrestrial Reference Frame 1993", ID["EPSG",1182]], MEMBER["European Terrestrial Reference Frame 1994", ID["EPSG",1183]], MEMBER["European Terrestrial Reference Frame 1996", ID["EPSG",1184]], MEMBER["European Terrestrial Reference Frame 1997", ID["EPSG",1185]], MEMBER["European Terrestrial Reference Frame 2000", ID["EPSG",1186]], MEMBER["European Terrestrial Reference Frame 2005", ID["EPSG",1204]], MEMBER["European Terrestrial Reference Frame 2014", ID["EPSG",1206]], ELLIPSOID["GRS 1980",6378137,298.2572221,LENGTHUNIT["metre",1,ID["EPSG",9001]],ID["EPSG",7019]], ENSEMBLEACCURACY[0.1],ID["EPSG",6258]],ID["EPSG",4258]],CONVERSION["UTM zone 30N",METHOD["Transverse Mercator",ID["EPSG",9807]],PARAMETER["Latitude of natural origin",0,ANGLEUNIT["degree",0.0174532925199433,ID["EPSG",9102]]],PARAMETER["Longitude of natural origin",-3,ANGLEUNIT["degree",0.0174532925199433,ID["EPSG",9102]]],PARAMETER["Scale factor at natural origin",0.9996,SCALEUNIT["unity",1,ID["EPSG",9201]]],PARAMETER["False easting",500000,LENGTHUNIT["metre",1,ID["EPSG",9001]]],PARAMETER["False northing",0,LENGTHUNIT["metre",1,ID["EPSG",9001]]],ID["EPSG",16030]],CS[Cartesian,2,ID["EPSG",4400]],AXIS["Easting (E)",east],AXIS["Northing (N)",north],LENGTHUNIT["metre",1,ID["EPSG",9001]],ID["EPSG",25830]])WKT";

	// A user-defined WKT2:2019 geographic CRS whose datum is an ensemble. The parser constructs this CRS from the WKT
	// elements because the CRS name has no catalogue match and the CRS has no authority identifier.
	const char* kWkt2GeographicEnsemble =
		R"WKT(GEOGCRS["Test ensemble GCS",
    ENSEMBLE["Test datum ensemble",
        MEMBER["Test realization 1"],
        MEMBER["Test realization 2"],
        ELLIPSOID["GRS 1980",6378137,298.257222101,LENGTHUNIT["metre",1]],
        ENSEMBLEACCURACY[0.1]],
    PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],
    CS[ellipsoidal,2],
        AXIS["geodetic latitude (Lat)",north],
        AXIS["geodetic longitude (Lon)",east],
        ANGLEUNIT["degree",0.0174532925199433]])WKT";

	// A user-defined WKT2:2019 vertical CRS whose datum is an ensemble.
	const char* kWkt2VerticalEnsemble =
		R"WKT(VERTCRS["Test ensemble height",
    ENSEMBLE["Test vertical datum ensemble",
        MEMBER["Test vertical realization 1"],
        MEMBER["Test vertical realization 2"],
        ENSEMBLEACCURACY[0.5]],
    CS[vertical,1],
        AXIS["gravity-related height (H)",up],
        LENGTHUNIT["metre",1]])WKT";

	// A WKT2:2019 engineering (local) CRS.
	const char* kWkt2Engineering =
		R"WKT(ENGCRS["A construction site CRS",
    EDATUM["P1"],
    CS[Cartesian,2],
        AXIS["site east (x)",east],
        AXIS["site north (y)",north],
        LENGTHUNIT["metre",1]])WKT";
}

// A WKT2 string is recognised by its root keyword and parsed into the right kind of CRS.
TEST(Wkt2, ParseWgs84Geographic)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Wgs84);
	auto const gcs = std::dynamic_pointer_cast<GeographicCoordinateSystem>(cs);
	ASSERT_TRUE(gcs);

	auto const ellipsoid = gcs->GetHorizontalDatum()->GetEllipsoid();
	EXPECT_NEAR(6378137.0, ellipsoid.GetSemiMajorAxis(), 1E-6);
	EXPECT_NEAR(298.257223563, ellipsoid.GetInverseFlattening(), 1E-9);
	EXPECT_NEAR(0.0, gcs->GetPrimeMeridian().GetLongitude(), 1E-12);

	EXPECT_EQ("EPSG", gcs->GetAuthority());
	EXPECT_EQ(4326, gcs->GetAuthorityCode());
}

// Selecting WKT2 emission turns GetWkt() into an ISO 19162 GEOGCRS.
TEST(Wkt2, EmitGeographicProducesGeogcrs)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Geographic);

	ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
	auto const wkt2 = cs->GetWkt();

	EXPECT_TRUE(wkt2.starts_with("GEOGCRS["));
	EXPECT_NE(std::string::npos, wkt2.find("DATUM[\"Test Datum\""));
	EXPECT_NE(std::string::npos, wkt2.find("ELLIPSOID[\"Test ellipsoid\""));
	EXPECT_NE(std::string::npos, wkt2.find("CS[ellipsoidal,2]"));
	EXPECT_NE(std::string::npos, wkt2.find("ANGLEUNIT[\"degree\""));
	// WKT1-only keywords must not leak into WKT2 output.
	EXPECT_EQ(std::string::npos, wkt2.find("GEOGCS["));
	EXPECT_EQ(std::string::npos, wkt2.find("SPHEROID["));
}

// Emit as WKT2, parse it back, and the result is structurally the same CRS.
TEST(Wkt2, RoundTripGeographicWkt2)
{
	auto const original = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Wgs84);

	std::string wkt2;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
		wkt2 = original->GetWkt();
	}

	auto const reparsed = GetCoordinateSystemFactory()->CreateFromWkt(wkt2);
	EXPECT_TRUE(AreEquivalent(original, reparsed));
}

// A CRS parsed from WKT1 and re-expressed as WKT2 round-trips to an equivalent CRS.
TEST(Wkt2, Wkt1ToWkt2IsEquivalent)
{
	auto const fromWkt1 = GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Geographic);

	std::string wkt2;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
		wkt2 = fromWkt1->GetWkt();
	}

	auto const fromWkt2 = GetCoordinateSystemFactory()->CreateFromWkt(wkt2);
	EXPECT_TRUE(AreEquivalent(fromWkt1, fromWkt2));
}

// GetWkt(WktVersion) emits the requested format without reading or mutating the default context's
// wktVersion, so a "load any version, store as another" conversion is explicit and does not touch shared state.
TEST(Wkt2, GetWktWithVersionIsPure)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Geographic);
	auto const before = GetDefaultContext()->wktVersion;

	EXPECT_TRUE(cs->GetWkt(WktVersion::Wkt1).starts_with("GEOGCS["));
	EXPECT_TRUE(cs->GetWkt(WktVersion::Wkt2_2015).starts_with("GEODCRS["));
	EXPECT_TRUE(cs->GetWkt(WktVersion::Wkt2_2019).starts_with("GEOGCRS["));

	// The global emission flag is untouched by the explicit-version calls.
	EXPECT_EQ(before, GetDefaultContext()->wktVersion);
}

// WKT2:2015 expresses a geographic CRS with the GEODCRS keyword (2019 uses GEOGCRS). The rest is the
// same, and the result still parses back to an equivalent CRS.
TEST(Wkt2, Emit2015GeographicUsesGeodcrs)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Geographic);

	std::string wkt2015;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2015};
		wkt2015 = cs->GetWkt();
	}

	EXPECT_TRUE(wkt2015.starts_with("GEODCRS["));
	EXPECT_EQ(std::string::npos, wkt2015.find("GEOGCRS["));
	EXPECT_NE(std::string::npos, wkt2015.find("CS[ellipsoidal,2]"));

	auto const reparsed = GetCoordinateSystemFactory()->CreateFromWkt(wkt2015);
	EXPECT_TRUE(AreEquivalent(cs, reparsed));
}

// WKT2:2015 expresses the base of a projected CRS with BASEGEODCRS (2019 uses BASEGEOGCRS).
TEST(Wkt2, Emit2015ProjectedUsesBaseGeodcrs)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Projected);

	std::string wkt2015;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2015};
		wkt2015 = cs->GetWkt();
	}

	EXPECT_TRUE(wkt2015.starts_with("PROJCRS["));
	EXPECT_NE(std::string::npos, wkt2015.find("BASEGEODCRS["));
	EXPECT_EQ(std::string::npos, wkt2015.find("BASEGEOGCRS["));

	auto const reparsed = GetCoordinateSystemFactory()->CreateFromWkt(wkt2015);
	EXPECT_TRUE(AreEquivalent(cs, reparsed));
}

// A WKT2 geocentric CRS round-trips through GEODCRS with a Cartesian 3D coordinate system.
TEST(Wkt2, RoundTripGeocentric)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Geocentric);
	auto const gcc = std::dynamic_pointer_cast<GeocentricCoordinateSystem>(cs);
	ASSERT_TRUE(gcc);
	EXPECT_NEAR(6378137.0, gcc->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), 1E-6);

	std::string wkt2;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
		wkt2 = cs->GetWkt();
	}
	EXPECT_TRUE(wkt2.starts_with("GEODCRS["));
	EXPECT_NE(std::string::npos, wkt2.find("CS[Cartesian,3]"));
	EXPECT_NE(std::string::npos, wkt2.find("geocentricX"));

	auto const reparsed = std::dynamic_pointer_cast<GeocentricCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt2));
	ASSERT_TRUE(reparsed);
	EXPECT_NEAR(6378137.0, reparsed->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), 1E-6);
}

// A WKT2 vertical CRS round-trips through VERTCRS with a CS[vertical,1].
TEST(Wkt2, RoundTripVertical)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Vertical);
	auto const vcs = std::dynamic_pointer_cast<VerticalCoordinateSystem>(cs);
	ASSERT_TRUE(vcs);

	std::string wkt2;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
		wkt2 = cs->GetWkt();
	}
	EXPECT_TRUE(wkt2.starts_with("VERTCRS["));
	EXPECT_NE(std::string::npos, wkt2.find("VDATUM[\"EGM2008 geoid\"]"));
	EXPECT_NE(std::string::npos, wkt2.find("CS[vertical,1]"));

	auto const reparsed = std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt2));
	ASSERT_TRUE(reparsed);
}

// A WKT2 compound CRS round-trips, with each component re-emitted as WKT2.
TEST(Wkt2, RoundTripCompound)
{
	std::string const compound = std::string("COMPOUNDCRS[\"Test compound\",") + kWkt2Utm31N + "," + kWkt2Vertical + "]";

	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(compound);
	auto const ccs = std::dynamic_pointer_cast<CompoundCoordinateSystem>(cs);
	ASSERT_TRUE(ccs);

	std::string wkt2;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
		wkt2 = cs->GetWkt();
	}
	EXPECT_TRUE(wkt2.starts_with("COMPOUNDCRS["));
	EXPECT_NE(std::string::npos, wkt2.find("PROJCRS["));
	EXPECT_NE(std::string::npos, wkt2.find("VERTCRS["));

	auto const reparsed = std::dynamic_pointer_cast<CompoundCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt2));
	ASSERT_TRUE(reparsed);
}

// A WKT2 engineering (local) CRS round-trips through ENGCRS.
TEST(Wkt2, RoundTripEngineering)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Engineering);
	auto const lcs = std::dynamic_pointer_cast<LocalCoordinateSystem>(cs);
	ASSERT_TRUE(lcs);

	std::string wkt2;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
		wkt2 = cs->GetWkt();
	}
	EXPECT_TRUE(wkt2.starts_with("ENGCRS["));
	EXPECT_NE(std::string::npos, wkt2.find("EDATUM[\"P1\"]"));
	EXPECT_NE(std::string::npos, wkt2.find("CS[Cartesian,2]"));

	auto const reparsed = std::dynamic_pointer_cast<LocalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt2));
	ASSERT_TRUE(reparsed);
}

// A WKT2 projected CRS parses into a ProjectedCoordinateSystem with the right method and parameters.
TEST(Wkt2, ParseProjectedUtm)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Utm31N);
	auto const pcs = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(cs);
	ASSERT_TRUE(pcs);

	auto const projection = pcs->GetProjection();
	EXPECT_EQ("Transverse_Mercator", projection->GetClassName());
	EXPECT_NEAR(3.0, std::get<double>(projection->GetParameter("central_meridian").GetValue()), 1E-9);
	EXPECT_NEAR(0.9996, std::get<double>(projection->GetParameter("scale_factor").GetValue()), 1E-12);
	EXPECT_NEAR(500000.0, std::get<double>(projection->GetParameter("false_easting").GetValue()), 1E-6);

	auto const base = std::dynamic_pointer_cast<GeographicCoordinateSystem>(pcs->GetGeographicCoordinateSystem());
	ASSERT_TRUE(base);
	EXPECT_NEAR(6378137.0, base->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), 1E-6);

	EXPECT_EQ("EPSG", pcs->GetAuthority());
	EXPECT_EQ(32631, pcs->GetAuthorityCode());
}

// Selecting WKT2 emission turns a projected CRS into a PROJCRS with BASEGEOGCRS + CONVERSION.
TEST(Wkt2, EmitProjectedProducesProjcrs)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Projected);

	ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
	auto const wkt2 = cs->GetWkt();

	EXPECT_TRUE(wkt2.starts_with("PROJCRS["));
	EXPECT_NE(std::string::npos, wkt2.find("BASEGEOGCRS["));
	EXPECT_NE(std::string::npos, wkt2.find("METHOD[\"Transverse Mercator\",ID[\"EPSG\",9807]]"));
	EXPECT_NE(std::string::npos, wkt2.find("PARAMETER[\"Longitude of natural origin\",3"));
	EXPECT_NE(std::string::npos, wkt2.find("CS[Cartesian,2]"));
	// The ellipsoid axes must not leak into the conversion as parameters.
	EXPECT_EQ(std::string::npos, wkt2.find("semi_major"));
	EXPECT_EQ(std::string::npos, wkt2.find("PROJCS["));
}

// Emit a projected CRS as WKT2, parse it back, and the result is structurally the same CRS.
TEST(Wkt2, RoundTripProjectedWkt2)
{
	auto const original = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Utm31N);

	std::string wkt2;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
		wkt2 = original->GetWkt();
	}

	auto const reparsed = GetCoordinateSystemFactory()->CreateFromWkt(wkt2);
	EXPECT_TRUE(AreEquivalent(original, reparsed));
}

// A projected CRS parsed from WKT1 and re-expressed as WKT2 round-trips to an equivalent CRS.
TEST(Wkt2, Wkt1ToWkt2ProjectedIsEquivalent)
{
	auto const fromWkt1 = GetCoordinateSystemFactory()->CreateFromWkt(kWkt1Projected);

	std::string wkt2;
	{
		ScopedWktVersion const scoped{WktVersion::Wkt2_2019};
		wkt2 = fromWkt1->GetWkt();
	}

	auto const fromWkt2 = GetCoordinateSystemFactory()->CreateFromWkt(wkt2);
	EXPECT_TRUE(AreEquivalent(fromWkt1, fromWkt2));
}

// CreateFromWkt parses a projected CRS with an ENSEMBLE base datum; AreEquivalent compares the result with EPSG:25830
// and returns true.
TEST(Wkt2, ParseProjectedWithDatumEnsemble)
{
	auto const coordinateSystem = GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Etrs89Utm30NEnsemble);
	auto const projectedCoordinateSystem = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(coordinateSystem);
	ASSERT_TRUE(projectedCoordinateSystem);

	auto const baseGeographicCoordinateSystem =
		std::dynamic_pointer_cast<GeographicCoordinateSystem>(projectedCoordinateSystem->GetGeographicCoordinateSystem());
	ASSERT_TRUE(baseGeographicCoordinateSystem);
	auto const datum = std::dynamic_pointer_cast<IInfo>(baseGeographicCoordinateSystem->GetHorizontalDatum());
	ASSERT_TRUE(datum);
	EXPECT_EQ("European Terrestrial Reference System 1989 ensemble", datum->GetName());
	// The ID of the ensemble (EPSG:6258), not the ID of its first MEMBER (EPSG:1178).
	EXPECT_EQ(6258, datum->GetAuthorityCode());
	EXPECT_NEAR(6378137.0, baseGeographicCoordinateSystem->GetHorizontalDatum()->GetEllipsoid().GetSemiMajorAxis(), 1E-6);

	EXPECT_EQ("EPSG", projectedCoordinateSystem->GetAuthority());
	EXPECT_EQ(25830, projectedCoordinateSystem->GetAuthorityCode());
	EXPECT_TRUE(AreEquivalent(coordinateSystem, GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(25830)));
}

// CreateFromWkt reads the geographic datum name and ellipsoid from ENSEMBLE.
TEST(Wkt2, ParseGeographicWithDatumEnsemble)
{
	auto const geographicCoordinateSystem =
		std::dynamic_pointer_cast<GeographicCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(kWkt2GeographicEnsemble));
	ASSERT_TRUE(geographicCoordinateSystem);

	auto const datum = std::dynamic_pointer_cast<IInfo>(geographicCoordinateSystem->GetHorizontalDatum());
	ASSERT_TRUE(datum);
	EXPECT_EQ("Test datum ensemble", datum->GetName());
	auto const ellipsoid = geographicCoordinateSystem->GetHorizontalDatum()->GetEllipsoid();
	EXPECT_NEAR(6378137.0, ellipsoid.GetSemiMajorAxis(), 1E-6);
	EXPECT_NEAR(298.257222101, ellipsoid.GetInverseFlattening(), 1E-9);
}

// CreateFromWkt uses the ENSEMBLE name as the vertical datum name.
TEST(Wkt2, ParseVerticalWithDatumEnsemble)
{
	auto const verticalCoordinateSystem =
		std::dynamic_pointer_cast<VerticalCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(kWkt2VerticalEnsemble));
	ASSERT_TRUE(verticalCoordinateSystem);

	auto const datum = std::dynamic_pointer_cast<IInfo>(verticalCoordinateSystem->GetVerticalDatum());
	ASSERT_TRUE(datum);
	EXPECT_EQ("Test vertical datum ensemble", datum->GetName());
}
