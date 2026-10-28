// The longitude of PRIMEM in Well-Known Text version 1 (WKT1) is in degrees: PrimeMeridian::FromWkt reads it in degrees,
// and the Geospatial Data Abstraction Library (GDAL) writes it in degrees when the GEOGCS unit is the grad. The Paris
// prime meridian is 2.5969213 grads, 2.33722917 degrees, east of Greenwich.

#include "TestBase.h"

#include <numbers>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	constexpr double kParisLongitudeInDegrees = 2.5969213 * 0.9;

	// Restores the parse policy of the default context when the scope ends.
	struct ScopedParsePolicy
	{
		WktParsePolicy previousParsePolicy{ GetDefaultContext()->parsePolicy };
		explicit ScopedParsePolicy(WktParsePolicy policy) { GetDefaultContext()->parsePolicy = policy; }
		~ScopedParsePolicy() { GetDefaultContext()->parsePolicy = previousParsePolicy; }
	};

	auto CalculatePrimeMeridianLongitudeInDegrees(IGeographicCoordinateSystem const& geographicSystem) -> double
	{
		auto const primeMeridian = geographicSystem.GetPrimeMeridian();
		return primeMeridian.GetLongitude() * primeMeridian.GetAngularUnit().GetRadiansPerUnit() * 180.0 / std::numbers::pi;
	}
}

// Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian (European Petroleum Survey Group (EPSG) code 4807,
// grads): CreateFromWkt with the TextWins policy, applied to the WKT1 that GetWkt returns, keeps the longitude of the
// prime meridian.
TEST(PrimeMeridianWkt1, GeographicCoordinateSystemInGradsPreservesParisPrimeMeridianLongitudeAfterWritingAndReadingWkt1)
{
	ScopedParsePolicy scopedTextWinsParsePolicy{ WktParsePolicy::TextWins };
	auto const wkt = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4807)->GetWkt();
	auto const geographicSystem = std::dynamic_pointer_cast<GeographicCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt));
	ASSERT_NE(nullptr, geographicSystem);
	EXPECT_NEAR(kParisLongitudeInDegrees, CalculatePrimeMeridianLongitudeInDegrees(*geographicSystem), 1e-9) << wkt;
}

// NTF (Paris) / Lambert zone II (EPSG code 27572): CreateFromWkt with the TextWins policy, applied to the WKT1 that GetWkt
// returns, keeps the longitude of the prime meridian of the geographic coordinate reference system.
TEST(PrimeMeridianWkt1, ProjectedCoordinateSystemPreservesParisPrimeMeridianLongitudeAfterWritingAndReadingWkt1)
{
	ScopedParsePolicy scopedTextWinsParsePolicy{ WktParsePolicy::TextWins };
	auto const wkt = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(27572)->GetWkt();
	auto const projectedSystem = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(GetCoordinateSystemFactory()->CreateFromWkt(wkt));
	ASSERT_NE(nullptr, projectedSystem);
	EXPECT_NEAR(kParisLongitudeInDegrees, CalculatePrimeMeridianLongitudeInDegrees(*projectedSystem->GetGeographicCoordinateSystem()), 1e-9) << wkt;
}