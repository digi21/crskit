# Python binding (pybind11).
#
# The crskit sources are compiled INTO the extension module instead of linking the shared
# library: the wheel is then a single self-contained _crskit.pyd / _crskit.so, with no STL
# crossing a DLL boundary and no crskit.dll to ship alongside it. CRSKIT_STATIC turns the
# _modeCrsKit attribute into nothing (see CrsKitExport.h).
#
# SQLite (the EPSG catalogue backend) is header-only in SqliteProvider.h, so the amalgamation
# is compiled in as well, exactly as the test executable does.

find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module)

# pybind11 comes from vcpkg or the system when available; otherwise from the pip package, which
# is how the wheel build (scikit-build-core) provides it.
find_package(pybind11 CONFIG QUIET)
if(NOT pybind11_FOUND)
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE PYBIND11_QUERY_RESULT)

    if(NOT PYBIND11_QUERY_RESULT EQUAL 0)
        message(FATAL_ERROR
            "pybind11 not found. Install it with: \"${Python_EXECUTABLE}\" -m pip install pybind11")
    endif()

    find_package(pybind11 CONFIG REQUIRED PATHS "${PYBIND11_CMAKE_DIR}" NO_DEFAULT_PATH)
endif()

pybind11_add_module(_crskit MODULE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/module.cpp"
    ${CRSKIT_SOURCES}
    "${CMAKE_SOURCE_DIR}/external/sqlite/sqlite3.c")

target_include_directories(_crskit PRIVATE
    "${CMAKE_SOURCE_DIR}/crskit"
    "${CMAKE_SOURCE_DIR}/external/sqlite")

target_compile_definitions(_crskit PRIVATE CRSKIT_STATIC _CRT_SECURE_NO_WARNINGS)

# scikit-build-core installs into the wheel root, so this lands the module inside the package.
install(TARGETS _crskit DESTINATION crskit)
