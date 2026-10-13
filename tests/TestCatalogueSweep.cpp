// Every CRS and every coordinate operation of the EPSG catalogue goes through the factory and the
// provider. Not part of the GIGS suite (no "Test5xxx" prefix).

#include <cstdlib>
#include <string>
#include <typeinfo>
#include <vector>

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

	// Rows of a query whose columns are all integers.
	auto IntegerRows(std::string const& sql) -> std::vector<std::vector<int>>
	{
		sqlite3* db{};
		if (sqlite3_open_v2(EpsgDatabasePath().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
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
				for (auto i = 0; i < sqlite3_column_count(stmt); i++)
					row.push_back(sqlite3_column_int(stmt, i));
			}
		}
		sqlite3_close(db);
		return rows;
	}
}

// Every operation of the catalogue reads its parameters: values with and without unit, and file references.
TEST(CatalogueSweep, EveryOperationReadsItsParameters)
{
	auto const provider = GetCurrentEnvironment()->Provider;
	auto const operations = IntegerRows("SELECT COORD_OP_CODE, COORD_OP_METHOD_CODE FROM epsg_coordoperation "
		"WHERE COORD_OP_METHOD_CODE IS NOT NULL ORDER BY COORD_OP_CODE");
	ASSERT_GT(operations.size(), 4000u);

	for (auto const& row : operations)
	{
		try
		{
			(void)provider->GetTransformationParameters(row[0], row[1]);
		}
		catch (std::exception const& e)
		{
			ADD_FAILURE() << "operation " << row[0] << " (method " << row[1] << "): " << typeid(e).name() << ": " << e.what();
		}
	}
}

// Operation 1450 (method 9620, Norway Offshore Interpolation) carries unitless values: two CRS codes and two
// operation names.
TEST(CatalogueSweep, UnitlessParameterKeepsItsValue)
{
	auto const parameters = GetCurrentEnvironment()->Provider->GetTransformationParameters(1450, 9620);
	ASSERT_EQ(4u, parameters.size());
	EXPECT_EQ(8047.0, std::get<double>(parameters[0]));
	EXPECT_EQ(8046.0, std::get<double>(parameters[1]));
	EXPECT_EQ("ED50 to WGS 84 (15)", std::get<std::string>(parameters[2]));
	EXPECT_EQ("ED50 to WGS 84 (14)", std::get<std::string>(parameters[3]));
}
