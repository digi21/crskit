// CompareTo of the datums: two datums with an authority code compare by authority and code; otherwise they compare by
// name, and whether CompareTo returns zero does not depend on the order of the operands.

#include "TestBase.h"
#include "../crskit/CoordinateSystems/HorizontalDatum.h"
#include "../crskit/CoordinateSystems/VerticalDatum.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	// This Well-Known Text 2 definition omits the ID element; the parsed datum has UnknownAuthorityCode and the ensemble name.
	char const* kWkt2Wgs84EnsembleWithoutId = R"WKT(GEOGCRS["WGS 84 without identifiers",
    ENSEMBLE["World Geodetic System 1984 ensemble",
        MEMBER["World Geodetic System 1984 (Transit)"],
        MEMBER["World Geodetic System 1984 (G730)"],
        ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]],
        ENSEMBLEACCURACY[2.0]],
    PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],
    CS[ellipsoidal,2],
        AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],
        AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]]])WKT";

	char const* kWkt2AlicanteWithoutId = R"WKT(VERTCRS["Alicante height without identifiers",
    VDATUM["Alicante"],
    CS[vertical,1],
        AXIS["gravity-related height (H)",up],
        LENGTHUNIT["metre",1]])WKT";

	auto GetHorizontalDatum(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<HorizontalDatum>
	{
		auto const geographicCoordinateSystem = std::dynamic_pointer_cast<GeographicCoordinateSystem>(coordinateSystem);
		return geographicCoordinateSystem ? std::dynamic_pointer_cast<HorizontalDatum>(geographicCoordinateSystem->GetHorizontalDatum()) : nullptr;
	}

	auto GetVerticalDatum(std::shared_ptr<CoordinateSystem> const& coordinateSystem) -> std::shared_ptr<VerticalDatum>
	{
		auto const verticalCoordinateSystem = std::dynamic_pointer_cast<VerticalCoordinateSystem>(coordinateSystem);
		return verticalCoordinateSystem ? std::dynamic_pointer_cast<VerticalDatum>(verticalCoordinateSystem->GetVerticalDatum()) : nullptr;
	}
}

TEST(DatumComparison, HorizontalDatumWithoutCodeMatchesTheCatalogueDatumOfTheSameNameInBothOrders)
{
	auto const horizontalDatumWithoutAuthorityCode = GetHorizontalDatum(GetCoordinateSystemFactory()->CreateFromWkt(kWkt2Wgs84EnsembleWithoutId));
	auto const catalogueHorizontalDatum = GetHorizontalDatum(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326));
	ASSERT_TRUE(horizontalDatumWithoutAuthorityCode);
	ASSERT_TRUE(catalogueHorizontalDatum);
	ASSERT_EQ(UnknownAuthorityCode, horizontalDatumWithoutAuthorityCode->GetAuthorityCode());
	ASSERT_EQ(6326, catalogueHorizontalDatum->GetAuthorityCode());

	EXPECT_EQ(0, catalogueHorizontalDatum->CompareTo(horizontalDatumWithoutAuthorityCode));
	EXPECT_EQ(0, horizontalDatumWithoutAuthorityCode->CompareTo(catalogueHorizontalDatum));
}

TEST(DatumComparison, HorizontalDatumsWithDifferentCodesDifferInBothOrders)
{
	auto const etrs89 = GetHorizontalDatum(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4258));
	auto const wgs84 = GetHorizontalDatum(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4326));
	ASSERT_TRUE(etrs89);
	ASSERT_TRUE(wgs84);

	EXPECT_NE(0, etrs89->CompareTo(wgs84));
	EXPECT_NE(0, wgs84->CompareTo(etrs89));
}

TEST(DatumComparison, VerticalDatumWithoutCodeMatchesTheCatalogueDatumOfTheSameNameInBothOrders)
{
	auto const verticalDatumWithoutAuthorityCode = GetVerticalDatum(GetCoordinateSystemFactory()->CreateFromWkt(kWkt2AlicanteWithoutId));
	auto const catalogueVerticalDatum = GetVerticalDatum(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(5782));
	ASSERT_TRUE(verticalDatumWithoutAuthorityCode);
	ASSERT_TRUE(catalogueVerticalDatum);
	ASSERT_EQ(UnknownAuthorityCode, verticalDatumWithoutAuthorityCode->GetAuthorityCode());
	ASSERT_EQ(5180, catalogueVerticalDatum->GetAuthorityCode());

	EXPECT_EQ(0, catalogueVerticalDatum->CompareTo(verticalDatumWithoutAuthorityCode));
	EXPECT_EQ(0, verticalDatumWithoutAuthorityCode->CompareTo(catalogueVerticalDatum));
}