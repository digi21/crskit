# crskit vcpkg port (registry mode)
# -----------------------------------------------------------------------------
# Builds crskit from a pinned release of this repository, consumed through the crskit vcpkg registry
# (this repo's ports/ + versions/). When hacking on the library itself, build it directly with CMake
# instead — this port is for CONSUMERS.
#
# To publish a new version: tag it (vX.Y.Z) and push, update the SHA512 below (vcpkg prints the expected
# SHA512 on the first, mismatching build), then run:
#   vcpkg x-add-version crskit --x-builtin-ports-root=ports --x-builtin-registry-versions-dir=versions

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO digi21/crskit
    REF "v${VERSION}"
    SHA512 bc6309d582c5ad48c8037c59d8d763a2cc51cd86093c805b12df20d2cf9f08191eabc4c9b05dd4971f88868a71e611c224a997763554a10339e555a8ab8fb47b
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
