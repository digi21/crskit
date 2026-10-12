// A failure of the EPSG database (locked, I/O error) must reach the caller as an exception, never read
// as "the code does not exist". Not part of the GIGS suite (no "Test5xxx" prefix).

#include <cstdlib>
#include <filesystem>
#include <format>
#include <memory>
#include <string>

#include "TestBase.h"
#include "../crskit/SqliteProvider.h"

using namespace CrsKit;

namespace
{
	auto EpsgDatabasePath() -> std::filesystem::path
	{
		if (char const* env = std::getenv("DIGI21_EPSG_SQLITE"))
			return env;
		return "C:\\ProgramData\\Digi3D.NET\\OpenGis\\epsg-v10_066.sqlite";
	}

	// Copies the EPSG database, makes the copy the current environment and holds an exclusive lock on it
	// from a second connection. The provider runs a query first, so the schema is already loaded and the
	// failure comes from sqlite3_step, not sqlite3_prepare.
	class LockedDatabase : public ::testing::Test
	{
	protected:
		std::shared_ptr<Environment> _original;
		std::shared_ptr<Epsg::SQliteProvider> _provider;
		std::filesystem::path _copy;
		sqlite3* _locker{};

		void SetUp() override
		{
			_original = GetCurrentEnvironment();

			auto const test = ::testing::UnitTest::GetInstance()->current_test_info()->name();
			_copy = std::filesystem::temp_directory_path() / std::format("crskit-locked-{}.sqlite", test);
			std::filesystem::copy_file(EpsgDatabasePath(), _copy, std::filesystem::copy_options::overwrite_existing);
			std::filesystem::permissions(_copy, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);

			_provider = std::make_shared<Epsg::SQliteProvider>(_copy.string());
			ASSERT_TRUE(Initialize(_provider));
			ASSERT_EQ("geographic 2D", GetCoordinateSystemAuthorityFactory()->GetKindOfCoordinateSystem(4258));

			ASSERT_EQ(SQLITE_OK, sqlite3_open_v2(_copy.string().c_str(), &_locker, SQLITE_OPEN_READWRITE, nullptr));
			ASSERT_EQ(SQLITE_OK, sqlite3_exec(_locker, "BEGIN EXCLUSIVE;", nullptr, nullptr, nullptr));
		}

		void TearDown() override
		{
			if (_locker)
			{
				sqlite3_exec(_locker, "ROLLBACK;", nullptr, nullptr, nullptr);
				sqlite3_close(_locker);
			}
			SetCurrentEnvironment(_original);
			_provider.reset();

			std::error_code ignored;
			std::filesystem::remove(_copy, ignored);
		}
	};
}

// An existing code on a locked database throws the SQLite error instead of AuthorityCodeNotFoundException.
TEST_F(LockedDatabase, CreateCoordinateSystemThrowsTheDatabaseError)
{
	try
	{
		(void)GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4258);
		FAIL() << "CreateCoordinateSystem(4258) did not throw on a locked database";
	}
	catch (AuthorityCodeNotFoundException const& e)
	{
		FAIL() << "a locked database was reported as a missing code: " << e.what();
	}
	catch (OpenGisException const& e)
	{
		EXPECT_NE(std::string::npos, std::string{ e.what() }.find("locked")) << e.what();
	}
}

TEST_F(LockedDatabase, KindOfCoordinateSystemThrows)
{
	EXPECT_THROW((void)GetCoordinateSystemAuthorityFactory()->GetKindOfCoordinateSystem(4326), OpenGisException);
}

TEST_F(LockedDatabase, UnitQueriesThrow)
{
	EXPECT_THROW((void)_provider->GetUnitIsLength(9001), OpenGisException);
	EXPECT_THROW((void)_provider->GetUnitIsAngle(9102), OpenGisException);
	EXPECT_THROW((void)_provider->GetFactorToStandardUnit(9001), OpenGisException);
}

TEST_F(LockedDatabase, EpsgVersionThrows)
{
	EXPECT_THROW((void)_provider->GetEpsgVersion(), OpenGisException);
}

// The candidate-operation list is read in a while (Step()) loop: a locked database must not return it empty.
TEST_F(LockedDatabase, OperationListThrows)
{
	EXPECT_THROW((void)_provider->OperationCodesToTransformSystemAtoB(4230, 4326), OpenGisException);
}

// The IUtilities functions return an empty string only for a missing code, never for a database failure.
TEST_F(LockedDatabase, UtilitiesThrowInsteadOfReturningEmpty)
{
	auto const u = GetUtilities();
	EXPECT_THROW((void)u->GetWktFromEpsgCode(4326), OpenGisException);
	EXPECT_THROW((void)u->GetWktFromEpsgCode(25830), OpenGisException);
	EXPECT_THROW((void)u->GetWktFromGeographicEpsgCode(4326, true), OpenGisException);
	EXPECT_THROW((void)u->GetWktFromEpsgCodeWithEastNorthAxes(5782), OpenGisException);
	EXPECT_THROW((void)u->GetCoordinateReferenceSystemName(4979), OpenGisException);
}

// Releasing the lock makes the same provider work again.
TEST_F(LockedDatabase, ProviderRecoversAfterTheLockIsReleased)
{
	sqlite3_exec(_locker, "ROLLBACK;", nullptr, nullptr, nullptr);
	EXPECT_EQ("ETRS89", GetCoordinateSystemAuthorityFactory()->CreateCoordinateSystem(4258)->GetName());
	EXPECT_TRUE(_provider->GetUnitIsLength(9001));
	EXPECT_FALSE(_provider->GetEpsgVersion().empty());
	EXPECT_FALSE(_provider->OperationCodesToTransformSystemAtoB(4230, 4326).empty());
}

// A unit that is not in the catalogue is neither a length nor an angle; that is not an error.
TEST(DatabaseNotFound, MissingUnitIsNeitherLengthNorAngle)
{
	auto const provider = GetCurrentEnvironment()->Provider;
	EXPECT_FALSE(provider->GetUnitIsLength(999999));
	EXPECT_FALSE(provider->GetUnitIsAngle(999999));
	EXPECT_TRUE(provider->GetUnitIsLength(9001));
	EXPECT_TRUE(provider->GetUnitIsAngle(9102));
}
