// StaticCoordinateTransformationFactory::CreateFromCoordinateSystems between two geographic coordinate reference systems
// (CRS) defined in Well-Known Text version 1 without authority code and without TOWGS84 parameters, with the same datum
// name and ellipsoid and different prime meridians: Nouvelle Triangulation Francaise (NTF) with the Paris prime meridian
// and angles in grads, and NTF with the Greenwich prime meridian and angles in degrees. The Paris meridian is 2.5969213
// grads, 2.33722917 degrees, east of Greenwich.

#include "TestBase.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;

namespace
{
	constexpr char const* kNtfParisWkt = R"WKT(GEOGCS["Test NTF (Paris)",DATUM["D_NTF",SPHEROID["Clarke_1880_IGN",6378249.2,293.4660212936269]],PRIMEM["Paris",2.33722917],UNIT["Grad",0.01570796326794897],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT";
	constexpr char const* kNtfGreenwichWkt = R"WKT(GEOGCS["Test NTF (Greenwich)",DATUM["D_NTF",SPHEROID["Clarke_1880_IGN",6378249.2,293.4660212936269]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433],AXIS["Latitude",NORTH],AXIS["Longitude",EAST]])WKT";

	constexpr double kParisLongitudeInDegrees = 2.33722917;

	auto CreateMathTransform(char const* sourceWkt, char const* targetWkt) -> std::shared_ptr<CoordinateTransformations::IMathTransform>
	{
		return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(GetCoordinateSystemFactory()->CreateFromWkt(sourceWkt), GetCoordinateSystemFactory()->CreateFromWkt(targetWkt))->GetMathTransform();
	}

	auto ExpectPointCoordinatesWithinTolerance(std::vector<double> const& expectedPoint, std::vector<double> const& point, double tolerance) -> void
	{
		ASSERT_EQ(expectedPoint.size(), point.size());
		for (std::size_t coordinateIndex = 0; coordinateIndex < point.size(); ++coordinateIndex)
			EXPECT_NEAR(expectedPoint[coordinateIndex], point[coordinateIndex], tolerance) << coordinateIndex;
	}
}

// The transformation from NTF (Paris) to NTF (Greenwich) converts grads to degrees and adds the longitude of the Paris
// meridian to the longitude.
TEST(PrimeMeridianChange, NtfParisToNtfGreenwichAddsTheLongitudeOfTheParisMeridian)
{
	ExpectPointCoordinatesWithinTolerance({ 46.8, 0.45 + kParisLongitudeInDegrees }, CreateMathTransform(kNtfParisWkt, kNtfGreenwichWkt)->Transform({ 52.0, 0.5 }), 1e-9);
}

// The transformation from NTF (Greenwich) to NTF (Paris) subtracts the longitude of the Paris meridian from the longitude
// and converts degrees to grads.
TEST(PrimeMeridianChange, NtfGreenwichToNtfParisSubtractsTheLongitudeOfTheParisMeridian)
{
	ExpectPointCoordinatesWithinTolerance({ 52.0, 0.5 }, CreateMathTransform(kNtfGreenwichWkt, kNtfParisWkt)->Transform({ 46.8, 0.45 + kParisLongitudeInDegrees }), 1e-9);
}
