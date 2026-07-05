# crskit vcpkg port (registry mode)
# -----------------------------------------------------------------------------
# Builds crskit from a pinned release of this repository, consumed through the crskit vcpkg registry
# (this repo's ports/ + versions/). When hacking on the library itself, build it directly with CMake
# instead — this port is for CONSUMERS.
#
# To publish a new version: tag it (vX.Y.Z) and push, update REF/SHA512 609e724f753153740da542cd06641a754bd8c8fa0e11f41f4b319a1c85b9f1f29428fee9428d96a5d2b5bbd5b250f4f4abe4787bd0cf100981cb25d5bf17ba7elow (vcpkg prints the expected
# SHA512 on the first, mismatching build), then run:
#   vcpkg x-add-version crskit --x-builtin-ports-root=ports --x-builtin-registry-versions-dir=versions

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO digi21/crskit
    REF "v${VERSION}"
    SHA512 609e724f753153740da542cd06641a754bd8c8fa0e11f41f4b319a1c85b9f1f29428fee9428d96a5d2b5bbd5b250f4f4abe4787bd0cf100981cb25d5bf17ba7e
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DDIGI21_BUILD_TESTS=OFF
        -DDIGI21_INSTALL=ON
)

vcpkg_cmake_install()

# Merge the debug/release CMake config into share/crskit and fix up the paths.
vcpkg_cmake_config_fixup(PACKAGE_NAME crskit CONFIG_PATH share/crskit)

# A shared library ships its headers once; the debug tree keeps only the .dll/.lib.
file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
