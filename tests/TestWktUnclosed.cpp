// A WKT whose last brackets are missing is parsed up to its end and no further. The parser read past the
// terminator of its buffer and wrote to the heap; an AddressSanitizer build reports it on the first run.
// Not part of the GIGS suite (no "Test5xxx" prefix).

#include <string>

#include "TestBase.h"

using namespace CrsKit;

namespace
{
	std::string const kGeographic =
		R"WKT(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433])WKT";
}

TEST(WktUnclosed, MissingLastBracketParsesToTheEnd)
{
	auto const cs = GetCoordinateSystemFactory()->CreateFromWkt(kGeographic);
	EXPECT_EQ("WGS 84", cs->GetName());
	EXPECT_EQ(2, cs->GetDimension());
}

TEST(WktUnclosed, EveryTruncationStaysInsideTheString)
{
	auto const complete = kGeographic + "]";
	for (auto length = 1u; length < complete.size(); ++length)
	{
		try
		{
			(void)GetCoordinateSystemFactory()->CreateFromWkt(complete.substr(0, length));
		}
		catch (std::exception const&)
		{
			// A truncated WKT may be rejected; it must not read outside the string.
		}
	}
}
