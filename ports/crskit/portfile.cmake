# crskit vcpkg port (registry mode)
# -----------------------------------------------------------------------------
# Builds crskit from a pinned release of this repository, consumed through the crskit vcpkg registry
# (this repo's ports/ + versions/). When hacking on the library itself, build it directly with CMake
# instead — this port is for CONSUMERS.
#
# To publish a new version: create the vX.Y.Z tag on the release commit and push that tag to the digi21/crskit
# repository on GitHub, update the SHA512 below (vcpkg prints the expected
# SHA512 on the first, mismatching build), then run:
#   vcpkg x-add-version crskit --x-builtin-ports-root=ports --x-builtin-registry-versions-dir=versions

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO digi21/crskit
    REF "v${VERSION}"
    SHA512 cf485820c7f11882c31302788ceabd3b0955bf1e0058be426694132e64de58832cd390245a80e64c9f96c92fa5d00fe2416ce1d8ea9d4c23ec1f150761e305d8
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
