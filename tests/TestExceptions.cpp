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
