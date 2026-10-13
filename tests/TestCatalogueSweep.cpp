// Every CRS and every coordinate operation of the EPSG catalogue goes through the factory and the
// provider. Not part of the GIGS suite (no "Test5xxx" prefix).

#include <cstdlib>
#include <iostream>
#include <map>
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

	// Kinds CreateCoordinateSystem builds from the code alone.
	auto SupportedCrsCodes() -> std::vector<int>
	{
		std::vector<int> codes;
		for (auto const& row : IntegerRows("SELECT COORD_REF_SYS_CODE FROM epsg_coordinatereferencesystem WHERE COORD_REF_SYS_KIND IN "
			"('projected','geographic 2D','geographic 3D','vertical','geocentric') ORDER BY COORD_REF_SYS_CODE"))
			codes.push_back(row[0]);
		return codes;
	}
}

// A CRS of a supported kind is either created, with a WKT and, in a sample, the name GetCoordinateReferenceSystemName
// returns, or rejected with UnsupportedFormatException naming what is missing. Any other exception fails.
TEST(CatalogueSweep, EveryCrsOfASupportedKindIsCreatedOrRejectedAsUnsupported)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const utilities = GetUtilities();
	auto const codes = SupportedCrsCodes();
	ASSERT_GT(codes.size(), 7000u);

	auto created = 0;
	std::map<std::string, std::vector<int>> rejected;
	for (auto const code : codes)
	{
		try
		{
			auto const crs = factory->CreateCoordinateSystem(code);
			EXPECT_FALSE(crs->GetWkt().empty()) << code;
			if (0 == created % 25)  // GetCoordinateReferenceSystemName creates the CRS again: a sample is enough.
				EXPECT_EQ(crs->GetName(), utilities->GetCoordinateReferenceSystemName(code)) << code;
			++created;
		}
		catch (UnsupportedFormatException const& e)
		{
			std::string const message{ e.what() };
			EXPECT_TRUE(message.contains("is not implemented") || message.contains("axes")) << code << ": " << message;
			EXPECT_THROW((void)utilities->GetCoordinateReferenceSystemName(code), UnsupportedFormatException) << code;
			rejected[message.substr(0, message.find(" of the conversion"))].push_back(code);
		}
		catch (std::exception const& e)
		{
			ADD_FAILURE() << "EPSG " << code << ": " << typeid(e).name() << ": " << e.what();
		}
	}

	auto total = 0u;
	for (auto const& [message, list] : rejected)
	{
		std::cout << "[ rejected ] " << list.size() << " CRS, first EPSG " << list.front() << ": " << message << '\n';
		total += static_cast<unsigned>(list.size());
	}
	std::cout << "[ created  ] " << created << " of " << codes.size() << " CRS, " << total << " rejected\n";
}

// The transform of a projection with no math transform behind it reports the projection name.
TEST(CatalogueSweep, TransformOfAnUnimplementedProjectionIsUnsupported)
{
	try
	{
		(void)GetMathTransformFactory()->CreateParameterizedTransform("No_Such_Projection", {});
		FAIL() << "no exception";
	}
	catch (UnsupportedFormatException const& e)
	{
		EXPECT_TRUE(std::string{ e.what() }.contains("No_Such_Projection")) << e.what();
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

// EPSG 9895 (LUREF / Luxembourg TM (3D)) is a projected CRS with 3 axes: rejected without an assert.
TEST(CatalogueSweep, ProjectedCrsWithThreeAxesIsUnsupported)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	EXPECT_THROW((void)factory->CreateCoordinateSystem(9895), UnsupportedFormatException);
	EXPECT_THROW((void)GetUtilities()->GetCoordinateReferenceSystemName(9895), UnsupportedFormatException);

	std::vector<CoordinateSystems::AxisInfo> const twoAxes{
		{ "X", CoordinateSystems::AxisOrientationEnum::East },
		{ "Y", CoordinateSystems::AxisOrientationEnum::North } };
	auto threeAxes = twoAxes;
	threeAxes.emplace_back("h", CoordinateSystems::AxisOrientationEnum::Up);
	EXPECT_THROW((void)factory->CreateProjectedCoordinateSystem(25830, threeAxes), UnsupportedFormatException);
}
// A derived vertical CRS whose base is derived too inherits the datum two levels up.
TEST(CatalogueSweep, VerticalCrsDerivedFromADerivedCrsGetsItsDatum)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	EXPECT_EQ(5103, std::dynamic_pointer_cast<CoordinateSystems::VerticalDatum>(factory->CreateVerticalCoordinateSystem(6358)->GetVerticalDatum())->GetAuthorityCode());
	EXPECT_EQ(5187, std::dynamic_pointer_cast<CoordinateSystems::VerticalDatum>(factory->CreateVerticalCoordinateSystem(5614)->GetVerticalDatum())->GetAuthorityCode());
	EXPECT_EQ("KOC WD depth (ft)", GetUtilities()->GetCoordinateReferenceSystemName(5614));
}
// Every operation of the catalogue whose method is implemented builds its projection, with a number in every
// numeric parameter, or is rejected with OpenGisException naming the parameter. The deprecated 1076 and 1077
// (Molodensky-Badekas) have no parameter values in the EPSG dataset.
TEST(CatalogueSweep, EveryOperationBuildsItsProjectionOrSaysWhy)
{
	auto const factory = GetCoordinateSystemAuthorityFactory();
	auto const operations = IntegerRows("SELECT COORD_OP_CODE, COORD_OP_METHOD_CODE FROM epsg_coordoperation "
		"WHERE COORD_OP_METHOD_CODE IS NOT NULL ORDER BY COORD_OP_CODE");

	auto built = 0;
	for (auto const& row : operations)
	{
		try
		{
			auto const projection = factory->CreateProjection(row[0], row[1]);
			for (auto const& parameter : projection->GetParameters())
				EXPECT_FALSE(parameter.GetName().empty()) << "operation " << row[0] << " has a parameter without name";
			++built;
		}
		catch (UnsupportedFormatException const&)
		{
		}
		catch (OpenGisException const& e)
		{
			EXPECT_TRUE(std::string{ e.what() }.contains("parameter")) << "operation " << row[0] << ": " << e.what();
			std::cout << "[ rejected ] operation " << row[0] << ": " << e.what() << '\n';
		}
		catch (std::exception const& e)
		{
			ADD_FAILURE() << "operation " << row[0] << " (method " << row[1] << "): " << typeid(e).name() << ": " << e.what();
		}
	}
	std::cout << "[ built    ] " << built << " of " << operations.size() << " operations\n";
}

TEST(CatalogueSweep, OperationWithoutParameterValuesIsRejected)
{
	for (auto const code : { 1076, 1077 })
	{
		try
		{
			(void)GetCoordinateSystemAuthorityFactory()->CreateProjection(code, 9636);
			ADD_FAILURE() << code << ": no exception";
		}
		catch (OpenGisException const& e)
		{
			EXPECT_TRUE(std::string{ e.what() }.contains("has no numeric value")) << code << ": " << e.what();
		}
	}
}