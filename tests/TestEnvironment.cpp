// Global GoogleTest environment: initializes the Digi21.OpenGis library with the
// EPSG authority provider (SQLite) once before running the tests.
//
// The path to the EPSG database can be overridden with the
// DIGI21_EPSG_SQLITE environment variable; otherwise the default location is used
// de Digi3D.NET.

#include <cstdlib>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "../crskit/CrsKit.h"
#include "../crskit/SqliteProvider.h"

namespace
{
	using namespace CrsKit;

	class Digi21OpenGisEnvironment : public ::testing::Environment
	{
	public:
		void SetUp() override
		{
			std::string ruta = "C:\\ProgramData\\Digi3D.NET\\OpenGis\\epsg-v10_066.sqlite";

			if (const char* env = std::getenv("DIGI21_EPSG_SQLITE"))
				ruta = std::string(env);

			Initialize(std::make_shared<Epsg::SQliteProvider>(ruta));

			// Data directory (NTv2/NadCon grids, geoids...) = folder of the sqlite.
			// The library opens the grids as the OpenGis directory + the file name.
			if (auto const pos = ruta.find_last_of("\\/"); pos != std::string::npos)
				GetDefaultContext()->dataDirectory = ruta.substr(0, pos + 1);
		}
	};

}

// main propio (no enlazamos gtest_main: vcpkg lo deja en lib/manual-link).
int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new Digi21OpenGisEnvironment());
	return RUN_ALL_TESTS();
}
