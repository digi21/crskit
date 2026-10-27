// GetCodeOfDatumAssociatedWithCrs: a projected or derived coordinate reference system (CRS) has no datum of its own in the catalogue and
// reports the datum of its base CRS. GetCodeOfPrimeMeridianAssociatedWithCrs returns the prime meridian code of the datum of the CRS.

#include "TestBase.h"

using namespace CrsKit;

TEST(DatumOfCrs, GeographicReportsItsOwnDatum)
{
	EXPECT_EQ(6258, GetCoordinateSystemAuthorityFactory()->GetCodeOfDatumAssociatedWithCrs(4258)); // ETRS89
}

TEST(DatumOfCrs, ProjectedReportsTheDatumOfItsBaseCrs)
{
	EXPECT_EQ(6258, GetCoordinateSystemAuthorityFactory()->GetCodeOfDatumAssociatedWithCrs(25830)); // ETRS89 / UTM zone 30N
	EXPECT_EQ(6230, GetCoordinateSystemAuthorityFactory()->GetCodeOfDatumAssociatedWithCrs(23030)); // ED50 / UTM zone 30N
}

TEST(DatumOfCrs, DerivedVerticalReportsTheDatumOfItsBaseChain)
{
	// 6358 NAVD88 depth (ftUS) -> 6360 NAVD88 height (ftUS) -> 5703 NAVD88 height, datum 5103.
	EXPECT_EQ(5103, GetCoordinateSystemAuthorityFactory()->GetCodeOfDatumAssociatedWithCrs(6358));
}

TEST(DatumOfCrs, PrimeMeridianIsTheOneOfTheDatumOfTheCrs)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	EXPECT_EQ(8901, factory->GetCodeOfPrimeMeridianAssociatedWithCrs(4258));  // ETRS89: Greenwich
	EXPECT_EQ(8901, factory->GetCodeOfPrimeMeridianAssociatedWithCrs(25830)); // ETRS89 / UTM zone 30N: Greenwich
	EXPECT_EQ(8903, factory->GetCodeOfPrimeMeridianAssociatedWithCrs(4807));  // NTF (Paris): Paris
	EXPECT_EQ(8903, factory->GetCodeOfPrimeMeridianAssociatedWithCrs(27572)); // NTF (Paris) / Lambert zone II: Paris
	EXPECT_EQ(0, factory->GetCodeOfPrimeMeridianAssociatedWithCrs(5782));     // Alicante height: a vertical datum has no prime meridian
}
