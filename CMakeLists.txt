cmake_minimum_required(VERSION 3.21)

project(crskit LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    # Sources are UTF-8 without BOM: cl.exe needs /utf-8 to read them as UTF-8 (not the ANSI codepage).
    # 4251/4275: STL/base types crossing the DLL boundary (benign in an all-MSVC ecosystem).
    # 4250: "inherits via dominance", inherent to the pure-interface hierarchy with virtual inheritance.
    add_compile_options(/utf-8 /wd4251 /wd4275 /wd4250)
endif()

# ---------------------------------------------------------------------------
# crskit library (shared)
# ---------------------------------------------------------------------------
file(GLOB_RECURSE CRSKIT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/crskit/*.cpp")

add_library(crskit SHARED ${CRSKIT_SOURCES})

target_include_directories(crskit PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/crskit>"
    "$<INSTALL_INTERFACE:include>")

# _CREATING_CRSKIT -> __declspec(dllexport) (see CrsKitExport.h). The library uses no TCHAR/wide APIs
# and no <windows.h>, so it does not define UNICODE/_UNICODE.
target_compile_definitions(crskit PRIVATE
    _CREATING_CRSKIT WIN32 _WINDOWS _USRDLL)

set_target_properties(crskit PROPERTIES
    DEBUG_POSTFIX "D"
    EXPORT_NAME crskit)

# ---------------------------------------------------------------------------
# Install / export: defines the CMake package 'crskit' consumed by the vcpkg port.
# Consumers do find_package(crskit CONFIG) and link crskit::crskit.
# ---------------------------------------------------------------------------
option(DIGI21_INSTALL "Generate the crskit package install/export rules" ON)

if(DIGI21_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS crskit
        EXPORT crskit-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

    # The whole crskit/*.h header tree is installed preserving its structure, because CrsKit.h includes
    # its subfolders (CoordinateSystems/..., CoordinateTransformations/...) by relative paths that must
    # resolve under the same include directory.
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/crskit/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING
        PATTERN "*.h")

    install(EXPORT crskit-targets
        FILE crskit-targets.cmake
        NAMESPACE crskit::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/crskit)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/crskit-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/crskit-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/crskit)

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/crskit-config.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/crskit)
endif()

# ---------------------------------------------------------------------------
# Python binding (pybind11). Off by default: the wheel build (scikit-build-core, see
# pyproject.toml) turns it on, and so can a plain CMake build with -DDIGI21_BUILD_PYTHON=ON.
# ---------------------------------------------------------------------------
option(DIGI21_BUILD_PYTHON "Build the Python binding (pybind11)" OFF)

if(DIGI21_BUILD_PYTHON)
    add_subdirectory(python)
endif()

# ---------------------------------------------------------------------------
# Tests (GoogleTest)
# ---------------------------------------------------------------------------
option(DIGI21_BUILD_TESTS "Build the tests (GoogleTest)" ON)

if(DIGI21_BUILD_TESTS)
    # gtest from the system/vcpkg if available; otherwise fetched (no sudo, self-contained for CI).
    find_package(GTest QUIET)
    if(NOT TARGET GTest::gtest)
        include(FetchContent)
        FetchContent_Declare(googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
    enable_testing()

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/Gigs/*.cpp")

    # SQLite (amalgamation) is vendored under external/sqlite.
    add_executable(crskit_tests
        ${TEST_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite/sqlite3.c")

    target_include_directories(crskit_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite")

    target_compile_definitions(crskit_tests PRIVATE
        _CONSOLE _CRT_SECURE_NO_WARNINGS GTEST_LINKED_AS_SHARED_LIBRARY=1 UNICODE _UNICODE)

    # Own main() in TestEnvironment.cpp -> do not link gtest_main.
    target_link_libraries(crskit_tests PRIVATE crskit GTest::gtest)

    # Windows only: copy gtest.dll + crskit(D).dll next to the exe. On Linux TARGET_RUNTIME_DLLS is
    # empty (the .so files resolve via rpath).
    if(WIN32)
        add_custom_command(TARGET crskit_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t "$<TARGET_FILE_DIR:crskit_tests>"
                    "$<TARGET_RUNTIME_DLLS:crskit_tests>"
            COMMAND_EXPAND_LISTS)
    endif()

    # EPSG database (not versioned). Override with -DDIGI21_EPSG_SQLITE=...
    set(DIGI21_EPSG_SQLITE "C:/ProgramData/Digi3D.NET/OpenGis/epsg-fiel.sqlite"
        CACHE FILEPATH "Path to the EPSG SQLite database used by the tests")

    add_test(NAME gigs COMMAND crskit_tests)
    set_tests_properties(gigs PROPERTIES
        ENVIRONMENT "DIGI21_EPSG_SQLITE=${DIGI21_EPSG_SQLITE}")
endif()
