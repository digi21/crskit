// Round trips through the transformations that CoordinateTransformationFactory::CreateFromCoordinateSystems builds
// between geographic 3D, vertical and compound coordinate reference systems (CRSs). The tests check whether applying each
// math transform and its inverse returns the starting point within 1 mm, or 1e-8 degrees for latitudes and longitudes.
// The Geospatial Integrity of Geoscience Software (GIGS) tests 5210 transform vertical CRSs in the direct direction only.
// The tests with a geoid are skipped when its grid is missing from the data directory: EGM08_REDNAP.txt (EPSG operation
// 9410), Und_min2.5x2.5_egm2008_isw=82_WGS84_TideFree (3858) and WW15MGH.GRD (10084).

#include "TestBase.h"
#include "RoundTrip.h"

#include <format>

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;
using namespace TestCrsKit;

namespace
{
	constexpr int kEtrs89_3D = 4937;
	constexpr int kWgs84_3D = 4979;
	constexpr int kAlicanteHeight = 5782;
	constexpr int kEgm2008Height = 3855;
	constexpr int kEgm96Height = 5773;

	auto CreateCatalogueCoordinateSystem(int code) -> std::shared_ptr<CoordinateSystem>
	{
		return GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code);
	}

	auto CreateCatalogueCompoundCoordinateSystem(int horizontalCode, int verticalCode) -> std::shared_ptr<CoordinateSystem>
	{
		return std::dynamic_pointer_cast<CoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateCompoundCoordinateSystem(horizontalCode, verticalCode));
	}

	// Creates the transformation from source to target with selectOperation returning operationCode, and expects that the
	// round trip of point returns point within the tolerances of the axes of pointCoordinateSystem. The helper keeps a
	// shared_ptr to the CoordinateTransformation until ExpectRoundTrip returns. Skips the test when a grid is missing.
	auto ExpectFactoryRoundTrip(std::shared_ptr<CoordinateSystem> const& source, std::shared_ptr<CoordinateSystem> const& target, int operationCode, std::vector<double> const& point, std::shared_ptr<CoordinateSystem> const& pointCoordinateSystem) -> void
	{
		CoordinateTransformationOptions options;
		options.selectOperation = [operationCode](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return operationCode; };

		std::shared_ptr<ICoordinateTransformation> transformation;
		try
		{
			transformation = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, target, options);
		}
		catch (GridFileNotFoundException const& exception)
		{
			GTEST_SKIP() << exception.what();
		}
		ExpectRoundTrip(transformation->GetMathTransform(), point, CalculateAxisTolerances(pointCoordinateSystem));
	}

	// Latitude, longitude (degrees) and height (metres) in Madrid.
	std::vector<double> const kGeographicPoint{ 40.4, -3.7, 700.0 };
	// Easting, northing and height (metres) in Madrid in Universal Transverse Mercator (UTM) zone 30N.
	std::vector<double> const kProjectedPoint{ 440000.0, 4475000.0, 650.0 };
}

// From geographic 3D to vertical and from vertical to geographic 3D, the point keeps its latitude and longitude in the
// axes of the geographic 3D CRS and the height changes between ellipsoidal and gravity-related.
TEST(VerticalRoundTrip, Geographic3DToAlicanteHeight)
{
	ExpectFactoryRoundTrip(CreateCatalogueCoordinateSystem(kEtrs89_3D), CreateCatalogueCoordinateSystem(kAlicanteHeight), 9410, kGeographicPoint, CreateCatalogueCoordinateSystem(kEtrs89_3D));
}

TEST(VerticalRoundTrip, AlicanteHeightToGeographic3D)
{
	ExpectFactoryRoundTrip(CreateCatalogueCoordinateSystem(kAlicanteHeight), CreateCatalogueCoordinateSystem(kEtrs89_3D), 9410, kGeographicPoint, CreateCatalogueCoordinateSystem(kEtrs89_3D));
}

TEST(VerticalRoundTrip, Geographic3DToEgm2008Height)
{
	ExpectFactoryRoundTrip(CreateCatalogueCoordinateSystem(kWgs84_3D), CreateCatalogueCoordinateSystem(kEgm2008Height), 3858, kGeographicPoint, CreateCatalogueCoordinateSystem(kWgs84_3D));
}

TEST(VerticalRoundTrip, Egm2008HeightToGeographic3D)
{
	ExpectFactoryRoundTrip(CreateCatalogueCoordinateSystem(kEgm2008Height), CreateCatalogueCoordinateSystem(kWgs84_3D), 3858, kGeographicPoint, CreateCatalogueCoordinateSystem(kWgs84_3D));
}

TEST(VerticalRoundTrip, CompoundWithAlicanteHeightToGeographic3D)
{
	auto const compound = CreateCatalogueCompoundCoordinateSystem(25830, kAlicanteHeight);
	ExpectFactoryRoundTrip(compound, CreateCatalogueCoordinateSystem(kEtrs89_3D), 9410, kProjectedPoint, compound);
}

TEST(VerticalRoundTrip, Geographic3DToCompoundWithAlicanteHeight)
{
	ExpectFactoryRoundTrip(CreateCatalogueCoordinateSystem(kEtrs89_3D), CreateCatalogueCompoundCoordinateSystem(25830, kAlicanteHeight), 9410, kGeographicPoint, CreateCatalogueCoordinateSystem(kEtrs89_3D));
}

// WGS 84 (EPSG 4326, axes latitude and longitude) + EGM2008 height.
TEST(VerticalRoundTrip, CompoundWithEgm2008HeightToGeographic3D)
{
	auto const compound = CreateCatalogueCompoundCoordinateSystem(4326, kEgm2008Height);
	ExpectFactoryRoundTrip(compound, CreateCatalogueCoordinateSystem(kWgs84_3D), 3858, kGeographicPoint, compound);
}

TEST(VerticalRoundTrip, CompoundWithEgm96HeightToGeographic3D)
{
	auto const compound = CreateCatalogueCompoundCoordinateSystem(32630, kEgm96Height);
	ExpectFactoryRoundTrip(compound, CreateCatalogueCoordinateSystem(kWgs84_3D), 10084, kProjectedPoint, compound);
}

TEST(VerticalRoundTrip, Geographic3DToCompoundWithEgm96Height)
{
	ExpectFactoryRoundTrip(CreateCatalogueCoordinateSystem(kWgs84_3D), CreateCatalogueCompoundCoordinateSystem(32630, kEgm96Height), 10084, kGeographicPoint, CreateCatalogueCoordinateSystem(kWgs84_3D));
}

// The caller keeps the math transform and releases the CoordinateTransformation that the factory returned before calling
// GetInverse.
TEST(VerticalRoundTrip, CompoundToGeographic3DAfterReleasingTheCoordinateTransformation)
{
	std::shared_ptr<IMathTransform> mathTransform;
	try
	{
		mathTransform = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(
			CreateCatalogueCompoundCoordinateSystem(25830, kAlicanteHeight), CreateCatalogueCoordinateSystem(kEtrs89_3D))->GetMathTransform();
	}
	catch (GridFileNotFoundException const& exception)
	{
		GTEST_SKIP() << exception.what();
	}
	ExpectRoundTrip(mathTransform, kProjectedPoint, CalculateAxisTolerances(CreateCatalogueCompoundCoordinateSystem(25830, kAlicanteHeight)));
}

// Height and depth of one vertical datum (5611 and 5706, Caspian Sea), vertical offsets between two vertical datums
// (5612 Baltic depth, 5705 Baltic height) and a change of unit within one vertical datum (5703 NAVD88 height in metres and
// 6360 in US survey feet).
TEST(VerticalRoundTrip, VerticalToVertical)
{
	for (auto const [sourceCode, targetCode] : { std::pair{ 5611, 5706 }, std::pair{ 5706, 5611 }, std::pair{ 5612, 5706 }, std::pair{ 5706, 5612 },
		std::pair{ 5611, 5705 }, std::pair{ 5705, 5611 }, std::pair{ 5703, 6360 }, std::pair{ 6360, 5703 } })
	{
		SCOPED_TRACE(std::format("EPSG {} to EPSG {}", sourceCode, targetCode));
		auto const source = CreateCatalogueCoordinateSystem(sourceCode);
		ExpectRoundTrip(GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, CreateCatalogueCoordinateSystem(targetCode))->GetMathTransform(),
			{ 100.0 }, CalculateAxisTolerances(source));
	}
}
