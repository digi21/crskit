// Angles in the EPSG unit "sexagesimal DMS" (9110, DDD.MMSSssssss) convert to decimal degrees exactly as written.
// Not part of the GIGS suite (no "Test5xxx" prefix).

#include <cmath>
#include <cstdlib>
#include <format>
#include <string>

#include "TestBase.h"
#include "../crskit/SqliteProvider.h"

using namespace CrsKit;

namespace
{
	auto EpsgDatabasePath() -> std::string
	{
		if (char const* env = std::getenv("DIGI21_EPSG_SQLITE"))
			return env;
		return "C:\\ProgramData\\Digi3D.NET\\OpenGis\\epsg-v10_066.sqlite";
	}

	// Reference conversion from the shortest decimal text of the double, which is the text the dataset holds.
	auto FromText(double value) -> double
	{
		auto const text = std::format("{}", std::abs(value));
		auto const point = text.find('.');
		auto const digits = (point == std::string::npos ? std::string{} : text.substr(point + 1)) + std::string(10, '0');
		auto const degrees = std::stod(text.substr(0, point));
		auto const minutes = std::stoi(digits.substr(0, 2));
		auto const seconds = std::stoll(digits.substr(2, 8)) / 1e6;
		auto const result = degrees + minutes / 60.0 + seconds / 3600.0;
		return value < 0 ? -result : result;
	}
}

TEST(SexagesimalDms, ValuesThatAreNotExactInBinary)
{
	auto const u = GetUtilities();
	EXPECT_NEAR(-(82 + 10 / 60.0), u->ToSexagesimal(9110, 0.0, -82.1), 1e-12);
	EXPECT_NEAR(-(91 + 7 / 60.0), u->ToSexagesimal(9110, 0.0, -91.07), 1e-12);
	EXPECT_NEAR(46 + 48 / 60.0, u->ToSexagesimal(9110, 0.0, 46.48), 1e-12);
	EXPECT_NEAR(45 + 54 / 60.0 + 0.356873 / 3600.0, u->ToSexagesimal(9110, 0.0, 45.5400356873), 1e-12);
	EXPECT_NEAR(-(0 + 30 / 60.0), u->ToSexagesimal(9110, 0.0, -0.3), 1e-12);
}

// Every DMS value of the catalogue.
TEST(SexagesimalDms, EveryCatalogueValue)
{
	sqlite3* db{};
	ASSERT_EQ(SQLITE_OK, sqlite3_open_v2(EpsgDatabasePath().c_str(), &db, SQLITE_OPEN_READONLY, nullptr));
	auto checked = 0;
	{
		Epsg::Stmt const stmt{ db, "SELECT COORD_OP_CODE, PARAMETER_VALUE FROM epsg_coordoperationparamvalue WHERE UOM_CODE=9110 AND PARAMETER_VALUE IS NOT NULL" };
		while (stmt.Step())
		{
			auto const value = sqlite3_column_double(stmt, 1);
			EXPECT_NEAR(FromText(value), GetUtilities()->ToSexagesimal(9110, 0.0, value), 1e-12) << "operation " << sqlite3_column_int(stmt, 0) << ": " << value;
			++checked;
		}
	}
	sqlite3_close(db);
	EXPECT_GT(checked, 4000);
}

// EPSG 2780 (NAD83(HARN) / Georgia East): longitude of origin -82.1 DMS = 82°10'W. Reference values from PROJ 9.8.1.
TEST(SexagesimalDms, GeorgiaEastProjectsLikeProj)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const t = GetCoordinateTransformationFactory()->CreateFromCoordinateSystems(factory->CreateCoordinateSystem(4152), factory->CreateCoordinateSystem(2780))->GetMathTransform();
	auto const p = t->Transform({ 27.0, -81.0 });
	EXPECT_NEAR(315790.52916947886, p[0], 1e-3);
	EXPECT_NEAR(-331914.6091739314, p[1], 1e-3);
}