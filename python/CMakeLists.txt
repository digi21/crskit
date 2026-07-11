# Python binding (pybind11).
#
# The module links the crskit SHARED library instead of compiling its sources in. That matters for
# the embedded interpreters: in the script window of Digi3D.NET or MDTopX the host application has
# already loaded crskit.dll, and the loader resolves a DLL by name, so the module binds to that very
# copy. Library state (the Environment: EPSG provider, factories, default CrsContext) is then shared
# with the host -- a script sees the CRS the application initialised, instead of running against a
# second, private copy of the library.
#
# The wheel therefore ships two binaries in the package: _crskit.pyd/.so and crskit.dll/libcrskit.so.
#
# SQLite (the EPSG catalogue backend) is header-only in SqliteProvider.h, so the amalgamation is
# compiled into the module, exactly as the test executable does.

find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module)

# pybind11 comes from vcpkg or the system when available; otherwise from the pip package, which is
# how the wheel build (scikit-build-core) provides it.
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
    "${CMAKE_SOURCE_DIR}/external/sqlite/sqlite3.c")

target_include_directories(_crskit PRIVATE "${CMAKE_SOURCE_DIR}/external/sqlite")
target_compile_definitions(_crskit PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(_crskit PRIVATE crskit)

# Linux: find libcrskit.so next to the module inside the package (Windows resolves it from the
# module's own folder, which crskit/__init__.py adds to the DLL search path).
set_target_properties(_crskit PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN")

# scikit-build-core installs into the wheel root, so both binaries land inside the package.
# The import library is of no use to a Python consumer, hence the separate, excluded component.
install(TARGETS _crskit LIBRARY DESTINATION crskit RUNTIME DESTINATION crskit)
install(TARGETS crskit
    RUNTIME DESTINATION crskit
    LIBRARY DESTINATION crskit
    ARCHIVE DESTINATION crskit COMPONENT development EXCLUDE_FROM_ALL)
