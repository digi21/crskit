#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Cut a new crskit release and update the vcpkg registry in one shot.
#
#   Usage:  tools/release.sh <version>        e.g.  tools/release.sh 0.2.0
#
# Preconditions:
#   - Your library changes are already committed on main and pushed (CI green).
#   - vcpkg is available (VCPKG_ROOT set, or `vcpkg` on PATH, or C:/vcpkg/vcpkg.exe).
#
# What it does:
#   1. Bumps the port version in ports/crskit/vcpkg.json and commits it.
#   2. Tags vX.Y.Z and pushes the tag (so GitHub generates the source tarball).
#   3. Computes the tarball SHA512 and writes it into the portfile, commits.
#   4. Runs `vcpkg x-add-version` to add the version to versions/, commits.
#   5. Pushes main.
# ---------------------------------------------------------------------------
set -euo pipefail

VERSION="${1:?usage: tools/release.sh <version>   (e.g. 0.2.0)}"
REPO="digi21/crskit"
cd "$(dirname "$0")/.."

# Locate vcpkg.
if [ -n "${VCPKG_ROOT:-}" ] && [ -x "${VCPKG_ROOT}/vcpkg" ]; then VCPKG="${VCPKG_ROOT}/vcpkg"
elif command -v vcpkg >/dev/null 2>&1; then VCPKG="vcpkg"
elif [ -x "/c/vcpkg/vcpkg.exe" ]; then VCPKG="/c/vcpkg/vcpkg.exe"
else echo "error: vcpkg not found (set VCPKG_ROOT)"; exit 1; fi

echo ">> Releasing crskit $VERSION"

# 1. Bump the port version and commit. Set "version" and drop any "port-version" (a real version bump
#    resets it); format-manifest then normalises the file. Uses sed so no Python is needed.
sed -i -E 's/("version"[[:space:]]*:[[:space:]]*)"[^"]*"/\1"'"$VERSION"'"/' ports/crskit/vcpkg.json
sed -i -E '/"port-version"[[:space:]]*:/d' ports/crskit/vcpkg.json
"$VCPKG" format-manifest ports/crskit/vcpkg.json
git add ports/crskit/vcpkg.json
git commit -m "vcpkg: bump crskit to $VERSION"

# 2. Tag and push (branch + tag).
git tag -a "v$VERSION" -m "crskit $VERSION"
git push origin HEAD
git push origin "v$VERSION"

# 3. Compute the source tarball SHA512 (retry: GitHub generates it on demand).
URL="https://github.com/$REPO/archive/refs/tags/v$VERSION.tar.gz"
echo ">> Fetching SHA512 of $URL"
SHA=""
for i in $(seq 1 10); do
  SHA=$(curl -fsSL "$URL" 2>/dev/null | sha512sum | cut -d' ' -f1)
  [ "${#SHA}" -eq 128 ] && [ "$SHA" != "$(printf '' | sha512sum | cut -d' ' -f1)" ] && break
  echo "   ...tarball not ready, retry $i"; sleep 5; SHA=""
done
[ -z "$SHA" ] && { echo "error: could not fetch tarball SHA512"; exit 1; }
echo "   SHA512 = $SHA"

# 4. Write the SHA512 into the portfile and commit.
sed -i -E "s|(SHA512 )[0-9a-f]+|\1$SHA|" ports/crskit/portfile.cmake
git add ports/crskit/portfile.cmake
git commit -m "vcpkg: pin the v$VERSION source SHA512 in the crskit port"

# 5. Add the version to the registry database and commit.
"$VCPKG" x-add-version crskit \
  --x-builtin-ports-root=ports \
  --x-builtin-registry-versions-dir=versions
git add versions
git commit -m "vcpkg: add crskit $VERSION to the registry version database"

# 6. Push.
git push origin HEAD
echo ">> Done. Consumers can bump their baseline to: $(git rev-parse HEAD)"
