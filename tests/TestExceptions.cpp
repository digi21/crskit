// The library throws typed exceptions (all derived from OpenGisException : std::runtime_error) so
// callers can discriminate failures by type, while existing catch(std::exception)/catch(OpenGisException)
// keeps working (backward compatible). Not part of the GIGS suite (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

// A non-existent EPSG code raises AuthorityCodeNotFoundException.
TEST(TypedExceptions, UnknownAuthorityCode)
{
	EXPECT_THROW((void)GetCoordinateSystemAuthorityFactory()->CreateLinearUnit(99999999), AuthorityCodeNotFoundException);
}

// A CRS code that is not in the catalogue raises AuthorityCodeNotFoundException too, rather than the
// bare runtime_error the kind dispatch used to throw for anything it did not recognize.
TEST(TypedExceptions, UnknownCrsCode)
{
	EXPECT_THROW((void)GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(999999), AuthorityCodeNotFoundException);
}

// A CRS that exists but whose kind this factory cannot build from a code alone (a compound CRS needs
// its two components) raises UnsupportedFormatException: a different failure from a missing code.
TEST(TypedExceptions, UnsupportedCrsKind)
{
	// EPSG 7415: Amersfoort / RD New + NAP height, a compound CRS.
	EXPECT_THROW((void)GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(7415), UnsupportedFormatException);
}

// Malformed WKT raises WktParseException.
TEST(TypedExceptions, MalformedWkt)
{
	EXPECT_THROW((void)GetCoordinateSystemFactory()->CreateFromWkt("this is not valid wkt"), WktParseException);
}

// Backward compatibility: the typed exceptions are still catchable through the base classes.
TEST(TypedExceptions, DerivedAreCatchableAsBase)
{
	EXPECT_THROW((void)GetCoordinateSystemAuthorityFactory()->CreateLinearUnit(99999999), OpenGisException);
	EXPECT_THROW((void)GetCoordinateSystemAuthorityFactory()->CreateLinearUnit(99999999), std::runtime_error);
	EXPECT_THROW((void)GetCoordinateSystemFactory()->CreateFromWkt("this is not valid wkt"), std::exception);
}
