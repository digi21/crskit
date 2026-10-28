// Round trips through the European Petroleum Survey Group (EPSG) catalogue from a point of the area of use: the
// transformations that CoordinateTransformationFactory::CreateFromCoordinateSystems creates between two coordinate
// reference systems (CRSs) of the catalogue, and the projected CRSs from their base geographic CRSs. Not part of the
// Geospatial Integrity of Geoscience Software (GIGS) suite (no "Test5xxx" prefix).
//
// GIGS 5101-5113 apply each projection and its inverse to the reference points of one CRS for each projection method,
// and GIGS 5201, 5203, 5204, 5207, 5208 and 5213 apply the geographic-geocentric conversion and operations 1314, 15929,
// 1803, 1313, 1763 and 1196, and their inverses, to reference points. GIGS 5210 transforms coordinates from the source
// vertical CRS to the target vertical CRS without applying the inverse transformation. For each catalogue entry that the
// factory builds, whose method is not excluded and whose direct transformation succeeds, the tests apply the
// transformation and its inverse and compare the returned coordinates with the starting coordinates within the tolerance
// of each axis; they do not compare with reference values.

#include <cmath>
#include <cstdlib>
#include <format>
#include <iostream>
#include <map>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "TestBase.h"
#include "RoundTrip.h"
#include "../crskit/SqliteProvider.h"

using namespace CrsKit;
using namespace CrsKit::CoordinateSystems;
using namespace CrsKit::CoordinateTransformations;
using namespace TestCrsKit;

namespace
{
	auto GetEpsgDatabaseFilePath() -> std::string
	{
		if (char const* epsgDatabaseFilePath = std::getenv("DIGI21_EPSG_SQLITE"))
			return epsgDatabaseFilePath;
		return "C:\\ProgramData\\Digi3D.NET\\OpenGis\\epsg-v10_066.sqlite";
	}

	// Rows of a query whose columns are all integers.
	auto QueryIntegerRows(std::string const& sql) -> std::vector<std::vector<int>>
	{
		sqlite3* db{};
		if (sqlite3_open_v2(GetEpsgDatabaseFilePath().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
		{
			sqlite3_close(db);
			throw std::runtime_error("cannot open the EPSG database");
		}

		std::vector<std::vector<int>> rows;
		{
			Epsg::Stmt const stmt{ db, sql };
			while (stmt.Step())
			{
				auto& row = rows.emplace_back();
				for (auto columnIndex = 0; columnIndex < sqlite3_column_count(stmt); columnIndex++)
					row.push_back(sqlite3_column_int(stmt, columnIndex));
			}
		}
		sqlite3_close(db);
		return rows;
	}

	// Methods excluded because EPSG marks them as not reversible (REVERSE_OP = 0) and catalogues the reverse direction as
	// another operation, with the reason of each exclusion. crskit does not implement any of the excluded methods.
	std::map<int, std::string_view> const kExclusionReasonsByMethod{
		{ 9617, "Madrid to ED50 polynomial: EPSG defines the coefficients for transforming coordinates from the source CRS to the target CRS only" },
		{ 9620, "Norway Offshore Interpolation: interpolates between two operations in one direction only" },
		{ 9634, "Maritime Provinces polynomial interpolation: EPSG defines the coefficients for transforming coordinates from the source CRS to the target CRS only" },
		{ 9645, "General polynomial of degree 2: EPSG defines the coefficients for transforming coordinates from the source CRS to the target CRS only" },
		{ 9648, "General polynomial of degree 6: EPSG defines the coefficients for transforming coordinates from the source CRS to the target CRS only" },
		{ 9652, "Complex polynomial of degree 3: EPSG defines the coefficients for transforming coordinates from the source CRS to the target CRS only" },
		{ 9653, "Complex polynomial of degree 4: EPSG defines the coefficients for transforming coordinates from the source CRS to the target CRS only" },
	};

	// Angular tolerance, in degrees, of the methods whose reverse computation does not invert the forward computation exactly.
	std::map<int, double> const kAngleTolerancesInDegreesByMethod{
		// Geographic 2D domain: the forward computation discards the ellipsoidal height in the target CRS, which the reverse takes as 0.
		{ 9603, 1e-6 }, { 9606, 1e-6 }, { 9607, 1e-6 }, { 9636, 1e-6 },
	};

	struct LatitudeLongitude
	{
		double latitude{};
		double longitude{};
	};

	// Starting position in the extent, with the latitude in degrees north of the equator and the longitude in degrees east
	// of Greenwich: the centre of the bounding box; empty when the extent has no bounding box. A box whose west bound
	// exceeds its east bound crosses the antimeridian. When the latitude and the longitude of the centre differ by less
	// than 1e-9 degrees, as at (0, 0), the function adds a quarter of the latitude span of the box to the latitude, so that,
	// for a box with a latitude span other than 0, the latitude and the longitude of the starting position differ.
	auto CalculateStartingPositionInExtent(int extentCode) -> std::optional<LatitudeLongitude>
	{
		auto bounds = GetCurrentEnvironment()->Provider->GetBounds(extentCode);
		auto const westSouth = bounds.GetMinCP();
		auto const eastNorth = bounds.GetMaxCP();
		if (westSouth.size() < 2 || eastNorth.size() < 2)
			return std::nullopt;

		auto longitude = (westSouth[0] + eastNorth[0]) / 2.0;
		if (westSouth[0] > eastNorth[0])
			longitude += longitude > 0.0 ? -180.0 : 180.0;
		auto latitude = (westSouth[1] + eastNorth[1]) / 2.0;
		if (std::fabs(latitude - longitude) < 1e-9)
			latitude += (eastNorth[1] - westSouth[1]) / 4.0;
		return LatitudeLongitude{ latitude, longitude };
	}

	// Ellipsoidal height of the starting point of a geographic 3D or geocentric CRS, and height of a vertical CRS, in the
	// unit of the CRS.
	constexpr auto kStartingHeight = 100.0;

	// The point at latitude and longitude in the axes and units of the CRS: latitude and longitude for a geographic CRS,
	// the projected coordinates for a projected CRS, X, Y and Z at the ellipsoidal height kStartingHeight above the ellipsoid
	// of the datum for a geocentric CRS, and kStartingHeight as the third coordinate of a geographic 3D CRS and as the
	// coordinate of a vertical CRS.
	auto CreatePointInCoordinateSystem(std::shared_ptr<CoordinateSystem> const& coordinateSystem, LatitudeLongitude const& position) -> std::vector<double>
	{
		if (auto const geographic = std::dynamic_pointer_cast<GeographicCoordinateSystem>(coordinateSystem))
		{
			auto const degreesPerUnit = geographic->GetAngularUnit().GetRadiansPerUnit() * 180.0 / std::numbers::pi;
			// position.longitude is relative to Greenwich; the longitude of the point in the geographic CRS is relative to the
			// prime meridian of the CRS. ToSexagesimal converts the longitude of the prime meridian to degrees, also from the
			// packed degrees, minutes and seconds of EPSG unit 9110.
			auto const primeMeridian = geographic->GetPrimeMeridian();
			auto const primeMeridianUnit = primeMeridian.GetAngularUnit();
			auto const longitude = position.longitude - GetUtilities()->ToSexagesimal(primeMeridianUnit.GetAuthorityCode(), primeMeridianUnit.GetRadiansPerUnit(), primeMeridian.GetLongitude());
			std::vector<double> point;
			for (auto axisIndex = 0; axisIndex < 2; ++axisIndex)
			{
				switch (geographic->GetAxis(axisIndex).GetOrientation())
				{
				case AxisOrientationEnum::North: point.push_back(position.latitude / degreesPerUnit); break;
				case AxisOrientationEnum::South: point.push_back(-position.latitude / degreesPerUnit); break;
				case AxisOrientationEnum::East: point.push_back(longitude / degreesPerUnit); break;
				case AxisOrientationEnum::West: point.push_back(-longitude / degreesPerUnit); break;
				default: throw std::runtime_error(std::format("axis {} of the geographic CRS is neither latitude nor longitude", axisIndex));
				}
			}
			if (3 == geographic->GetDimension())
				point.push_back(kStartingHeight);
			return point;
		}
		if (auto const projected = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(coordinateSystem))
		{
			auto const geographicCoordinateSystem = std::dynamic_pointer_cast<CoordinateSystem>(projected->GetGeographicCoordinateSystem());
			return GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geographicCoordinateSystem, projected)->GetMathTransform()->Transform(
				CreatePointInCoordinateSystem(geographicCoordinateSystem, position));
		}
		if (auto const geocentric = std::dynamic_pointer_cast<GeocentricCoordinateSystem>(coordinateSystem))
		{
			auto const ellipsoid = geocentric->GetHorizontalDatum()->GetEllipsoid();
			auto const ellipsoidMetresPerUnit = ellipsoid.GetLinearUnit().GetMetersPerUnit();
			auto const semiMajorAxis = ellipsoid.GetSemiMajorAxis() * ellipsoidMetresPerUnit;
			auto const semiMinorAxis = ellipsoid.GetSemiMinorAxis() * ellipsoidMetresPerUnit;
			auto const eccentricitySquared = 1.0 - semiMinorAxis * semiMinorAxis / (semiMajorAxis * semiMajorAxis);
			auto const latitude = position.latitude * std::numbers::pi / 180.0;
			auto const longitude = position.longitude * std::numbers::pi / 180.0;
			auto const metresPerUnit = geocentric->GetLinearUnit().GetMetersPerUnit();
			auto const height = kStartingHeight * metresPerUnit;
			auto const primeVerticalRadius = semiMajorAxis / std::sqrt(1.0 - eccentricitySquared * std::sin(latitude) * std::sin(latitude));
			return { (primeVerticalRadius + height) * std::cos(latitude) * std::cos(longitude) / metresPerUnit,
				(primeVerticalRadius + height) * std::cos(latitude) * std::sin(longitude) / metresPerUnit,
				(primeVerticalRadius * (1.0 - eccentricitySquared) + height) * std::sin(latitude) / metresPerUnit };
		}
		if (std::dynamic_pointer_cast<VerticalCoordinateSystem>(coordinateSystem))
			return { kStartingHeight };
		throw std::runtime_error(std::format("{} is neither geographic, projected, geocentric nor vertical", coordinateSystem->GetName()));
	}

	// Catalogue entries that the sweep does not round-trip, grouped by reason. For each reason, _entriesByReason stores the
	// codes of the entries and the detail of the first entry, such as the message of an exception.
	class EntriesWithoutRoundTrip
	{
		struct Entries
		{
			std::vector<int> codes;
			std::string firstEntryDetail;
		};
		std::map<std::string, Entries> _entriesByReason;

	public:
		auto Add(std::string const& reason, int code, std::string const& detail = {}) -> void
		{
			auto& entries = _entriesByReason[reason];
			if (entries.codes.empty())
				entries.firstEntryDetail = detail;
			entries.codes.push_back(code);
		}

		auto Print() const -> void
		{
			for (auto const& [reason, entries] : _entriesByReason)
			{
				std::string codeList;
				for (auto const code : entries.codes)
					codeList += std::format(" {}", code);
				std::cout << std::format("[ not round-tripped ] {}, {} entries:{}", reason, entries.codes.size(), codeList);
				if (!entries.firstEntryDetail.empty())
					std::cout << std::format("; first entry: {}", entries.firstEntryDetail);
				std::cout << '\n';
			}
		}
	};

	// First line of a text.
	auto ExtractFirstLine(std::string const& text) -> std::string
	{
		return text.substr(0, text.find('\n'));
	}

	// Cache of the catalogue CRSs, which many operations share.
	class CatalogueCoordinateSystems
	{
		std::map<int, std::shared_ptr<CoordinateSystem>> _coordinateSystemsByCode;

	public:
		auto GetOrCreate(int code) -> std::shared_ptr<CoordinateSystem>
		{
			auto& coordinateSystem = _coordinateSystemsByCode[code];
			if (nullptr == coordinateSystem)
				coordinateSystem = GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(code);
			return coordinateSystem;
		}
	};
}

// Every non-deprecated transformation of the catalogue between two geographic 2D CRSs, between two geocentric CRSs, from a
// geographic 3D CRS to a vertical CRS, or between two vertical CRSs, with selectOperation returning the code of the
// transformation when the factory offers several. The factory replaces a geographic 3D CRS by its geographic 2D CRS when
// it looks for an operation between geographic CRSs, derives the transformation between projected CRSs from the
// operations between their base geographic CRSs, and resolves a compound CRS component by component, so the test leaves
// out the catalogued operations between two geographic 3D CRSs, between two projected CRSs and to or from a compound CRS.
// An operation that the factory does not build or replaces, whose grid is missing or whose direct transformation throws
// at the starting position is listed at the end, not counted as a failure.
TEST(RoundTripSweep, CatalogueTransformationRoundTripsMatchStartingCoordinatesWithinAxisTolerances)
{
	auto const transformations = QueryIntegerRows(
		"SELECT o.COORD_OP_CODE, o.COORD_OP_METHOD_CODE, o.SOURCE_CRS_CODE, o.TARGET_CRS_CODE FROM epsg_coordoperation o "
		"JOIN epsg_coordinatereferencesystem s ON s.COORD_REF_SYS_CODE = o.SOURCE_CRS_CODE "
		"JOIN epsg_coordinatereferencesystem t ON t.COORD_REF_SYS_CODE = o.TARGET_CRS_CODE "
		"WHERE o.DEPRECATED = 0 AND o.COORD_OP_TYPE = 'transformation' AND ("
		"(s.COORD_REF_SYS_KIND = 'geographic 2D' AND t.COORD_REF_SYS_KIND = 'geographic 2D') OR "
		"(s.COORD_REF_SYS_KIND = 'geocentric' AND t.COORD_REF_SYS_KIND = 'geocentric') OR "
		"(s.COORD_REF_SYS_KIND = 'geographic 3D' AND t.COORD_REF_SYS_KIND = 'vertical') OR "
		"(s.COORD_REF_SYS_KIND = 'vertical' AND t.COORD_REF_SYS_KIND = 'vertical')) "
		"ORDER BY o.COORD_OP_CODE");
	ASSERT_GT(transformations.size(), 2000u);

	auto const provider = GetCurrentEnvironment()->Provider;
	CatalogueCoordinateSystems coordinateSystems;
	EntriesWithoutRoundTrip entriesWithoutRoundTrip;
	std::map<int, int> roundTripCountsByMethod;
	std::map<int, int> failureCountsByMethod;

	for (auto const& row : transformations)
	{
		auto const operationCode = row[0];
		auto const methodCode = row[1];
		auto const sourceCode = row[2];
		auto const targetCode = row[3];

		if (auto const exclusion = kExclusionReasonsByMethod.find(methodCode); kExclusionReasonsByMethod.end() != exclusion)
		{
			entriesWithoutRoundTrip.Add(std::format("method {} excluded, {}", methodCode, exclusion->second), operationCode);
			continue;
		}

		std::shared_ptr<CoordinateSystem> source;
		std::shared_ptr<ICoordinateTransformation> transformation;
		std::vector<double> startingPoint;
		try
		{
			auto const startingPosition = CalculateStartingPositionInExtent(provider->GetCoordinateOperationAreaOfUse(operationCode));
			if (!startingPosition)
			{
				entriesWithoutRoundTrip.Add("the area of use has no bounding box", operationCode);
				continue;
			}
			source = coordinateSystems.GetOrCreate(sourceCode);
			CoordinateTransformationOptions options;
			options.selectOperation = [operationCode](std::string const&, std::string const&, std::vector<CoordinateOperation> const&) { return operationCode; };
			transformation = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(source, coordinateSystems.GetOrCreate(targetCode), options);
			startingPoint = CreatePointInCoordinateSystem(source, *startingPosition);
		}
		catch (GridFileNotFoundException const& exception)
		{
			entriesWithoutRoundTrip.Add(std::format("grid {} missing", exception.gridFileName), operationCode);
			continue;
		}
		catch (std::exception const& exception)
		{
			entriesWithoutRoundTrip.Add(std::format("method {}, exception while creating the CRSs, the transformation or the starting point, {}", methodCode, typeid(exception).name()), operationCode, ExtractFirstLine(exception.what()));
			continue;
		}

		// The authority code is empty when the factory combines the operation with other transformation steps.
		if (auto const appliedCode = transformation->GetAuthorityCode(); !appliedCode.empty() && std::to_string(operationCode) != appliedCode)
		{
			entriesWithoutRoundTrip.Add(std::format("method {}, the transformation that the factory created has the authority code of another operation", methodCode), operationCode, std::format("operation {}", appliedCode));
			continue;
		}

		auto const roundTrip = ApplyTransformAndInverse(transformation->GetMathTransform(), startingPoint);
		if (roundTrip.transformedPoint.empty())
		{
			entriesWithoutRoundTrip.Add(std::format("method {}, the direct transformation threw", methodCode), operationCode, ExtractFirstLine(roundTrip.exceptionDescription));
			continue;
		}

		++roundTripCountsByMethod[methodCode];
		auto const methodTolerance = kAngleTolerancesInDegreesByMethod.find(methodCode);
		auto const axisTolerances = CalculateAxisTolerances(source, kAngleTolerancesInDegreesByMethod.end() == methodTolerance ? 1e-8 : methodTolerance->second);
		if (roundTrip.exceptionDescription.empty() && IsWithinTolerances(startingPoint, roundTrip.returnedPoint, axisTolerances))
			continue;

		++failureCountsByMethod[methodCode];
		ADD_FAILURE() << std::format("operation {} (method {}), EPSG {} to EPSG {}: ", operationCode, methodCode, sourceCode, targetCode)
			<< DescribeRoundTrip(startingPoint, roundTrip, axisTolerances);
	}

	entriesWithoutRoundTrip.Print();
	for (auto const& [methodCode, roundTripCount] : roundTripCountsByMethod)
		std::cout << std::format("[ method {:5} ] {} round trips, {} failed\n", methodCode, roundTripCount, failureCountsByMethod[methodCode]);
}

// Every non-deprecated projected CRS of the catalogue, from the starting position in its area of use, in its base
// geographic CRS, to the projected CRS and then back to the base geographic CRS. A CRS that the factory does not build
// or whose projection throws at that point is listed at the end, not counted as a failure.
TEST(RoundTripSweep, ProjectionRoundTripsMatchStartingCoordinatesWithinAxisTolerances)
{
	auto const projectedCodes = QueryIntegerRows(
		"SELECT COORD_REF_SYS_CODE FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_KIND = 'projected' AND DEPRECATED = 0 ORDER BY COORD_REF_SYS_CODE");
	ASSERT_GT(projectedCodes.size(), 4000u);

	auto const provider = GetCurrentEnvironment()->Provider;
	EntriesWithoutRoundTrip entriesWithoutRoundTrip;
	auto roundTripCount = 0;
	auto failureCount = 0;

	for (auto const& row : projectedCodes)
	{
		auto const projectedCode = row[0];

		std::shared_ptr<ProjectedCoordinateSystem> projected;
		std::shared_ptr<CoordinateSystem> geographic;
		std::shared_ptr<ICoordinateTransformation> transformation;
		std::vector<double> startingPoint;
		try
		{
			auto const startingPosition = CalculateStartingPositionInExtent(provider->GetAreaOfUseCodeForCrs(projectedCode));
			if (!startingPosition)
			{
				entriesWithoutRoundTrip.Add("the area of use has no bounding box", projectedCode);
				continue;
			}
			projected = std::dynamic_pointer_cast<ProjectedCoordinateSystem>(GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(projectedCode));
			geographic = std::dynamic_pointer_cast<CoordinateSystem>(projected->GetGeographicCoordinateSystem());
			transformation = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(geographic, projected);
			startingPoint = CreatePointInCoordinateSystem(geographic, *startingPosition);
		}
		catch (std::exception const& exception)
		{
			entriesWithoutRoundTrip.Add(std::format("exception while creating the projected CRS, the transformation or the starting point, {}", typeid(exception).name()), projectedCode, ExtractFirstLine(exception.what()));
			continue;
		}

		auto const roundTrip = ApplyTransformAndInverse(transformation->GetMathTransform(), startingPoint);
		if (roundTrip.transformedPoint.empty())
		{
			entriesWithoutRoundTrip.Add("the projection threw", projectedCode, ExtractFirstLine(roundTrip.exceptionDescription));
			continue;
		}

		++roundTripCount;
		auto const axisTolerances = CalculateAxisTolerances(geographic);
		if (roundTrip.exceptionDescription.empty() && IsWithinTolerances(startingPoint, roundTrip.returnedPoint, axisTolerances))
			continue;

		++failureCount;
		ADD_FAILURE() << std::format("projected CRS EPSG {} ({}), from EPSG {}: ", projectedCode, projected->GetProjection()->GetClassName(), geographic->GetAuthorityCode())
			<< DescribeRoundTrip(startingPoint, roundTrip, axisTolerances);
	}

	entriesWithoutRoundTrip.Print();
	std::cout << std::format("[ projected  ] {} round trips, {} failed\n", roundTripCount, failureCount);
}
