// A WKT without a root element is rejected with WktParseException instead of dereferencing a null node.
// Not part of the GIGS suite (no "Test5xxx" prefix).

#include "TestBase.h"

using namespace CrsKit;

TEST(WktMalformed, TextWithoutRootElementThrows)
{
	for (auto const* wkt : { "]", ",", "]]", ",,", "abc,", "abc]", "abc", "", "   " })
		EXPECT_THROW((void)GetCoordinateSystemFactory()->CreateFromWkt(wkt), WktParseException) << '"' << wkt << '"';
}