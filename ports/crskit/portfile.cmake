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
    SHA512 96a091e62be392db7a16ed3c2081c9b4f3fe430db0609e0f94a3341b500a04b58bb725312e9b9333254eb02f60b743e59398dec3a8fff59f44ed0907cef3b5fd
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
