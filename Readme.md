<img src="branding/crskit-logo-256.png" alt="CrsKit" width="110" align="right">

# CrsKit

A modern C++23 geodesy library for coordinate reference systems, WKT and coordinate transformations.

[![CI](https://github.com/digi21/crskit/actions/workflows/ci.yml/badge.svg)](https://github.com/digi21/crskit/actions/workflows/ci.yml)
![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C)
![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-informational)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue)](LICENSE)

---

**CrsKit** implements the OGC *Coordinate Transformation Services* model in modern, portable C++.
It reads and writes coordinate reference systems, parses and emits Well-Known Text, and builds and
applies transformations between systems, backed by the full EPSG catalogue.

## Features

- **Coordinate reference systems** — geographic 2D/3D, projected, vertical, geocentric and compound,
  created by EPSG code or from WKT.
- **Well-Known Text** — parse and emit **WKT 1** (OGC and ESRI dialects) and **WKT 2** (ISO 19162:2015
  and :2019), with grammar auto-detection on parse.
- **Coordinate transformations** — datum shifts (geocentric / Molodensky-Badekas / Position-Vector /
  Coordinate-Frame, TOWGS84), map projections, and a large set of **grid-based** operations: NTv2,
  NADCON, and geoid models in EGM2008 / EGM96, GeoTIFF (`.tif`, the PROJ/GDAL format), `.gtx`, and
  AUSGeoid (`.gsb`) — with a self-contained DEFLATE decoder, so the library has no heavy dependencies.
- **Conformance-tested** — a port of the IOGP **GIGS** (Geospatial Integrity of Geoscience Software)
  test series validates the geodesy against the reference dataset.
- **Portable and thread-safe** — C++23, no MFC or `windows.h`; builds and passes its tests on Windows
  (MSVC) and Linux (GCC), and is safe to use from multiple threads.

## Using CrsKit (vcpkg)

CrsKit is distributed as a vcpkg registry. Point your project at it in `vcpkg-configuration.json`:

```json
{
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/digi21/crskit",
      "baseline": "<commit of this repo's main>",
      "packages": ["crskit"]
    }
  ]
}
```

add `crskit` to your `vcpkg.json`, and consume it from CMake:

```cmake
find_package(crskit CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE crskit::crskit)
```

## Using CrsKit from Python

CrsKit has a Python binding (pybind11), published as a wheel:

```python
import crskit

crskit.init("epsg.sqlite")
etrs89 = crskit.crs_from_epsg(4258)      # geographic 2D (latitude, longitude)
utm30n = crskit.crs_from_epsg(25830)     # ETRS89 / UTM zone 30N

crskit.transformation(etrs89, utm30n).transform([40.416775, -3.703790])
```

It transforms whole NumPy arrays in a single call, emits WKT 1 and WKT 2, compares systems by their
mathematical definition, and ships type stubs. There is a [tutorial](python/TUTORIAL.md) that starts
from what a coordinate reference system even is, a [reference](python/README.md), and
[runnable examples](python/examples/).

## Building from source

```bash
cmake -S . -B build
cmake --build build
```

The only build inputs are a C++23 compiler and CMake — SQLite is vendored and GoogleTest is fetched
automatically. Visual Studio opens the `CMakeLists.txt` directly (Open Folder), or generate a solution
with `cmake -G "Visual Studio 17 2022" -B build`.

### Running the tests

The tests read the EPSG SQLite database, which is not shipped with the source (it is the IOGP EPSG
Geodetic Parameter Dataset). Get it either way:

- **Download** a prebuilt copy from the [`epsg-data`](https://github.com/digi21/crskit/releases/tag/epsg-data)
  release — `epsg-vX.Y.sqlite`, named after the EPSG dataset version.
- **Build** it yourself from the EPSG SQL scripts with [`tools/epsg-sqlite`](tools/epsg-sqlite/) (see its
  README) — useful for a newer EPSG version than the one published there.

Then point `DIGI21_EPSG_SQLITE` at it and run:

```bash
DIGI21_EPSG_SQLITE=/path/to/epsg.sqlite ctest --test-dir build --output-on-failure
```

A few tests need NTv2/geoid grid files that cannot be redistributed (see *Data and attribution*); they
skip or are excluded when the grids are absent.

## Data and attribution

CrsKit is the **software**; the coordinate reference *data* are supplied separately.

This product uses the **EPSG Geodetic Parameter Dataset** (<https://epsg.org>), owned by the
International Association of Oil & Gas Producers (IOGP) and used under the EPSG Terms of Use
(<https://epsg.org/terms-of-use.html>). The EPSG Dataset is provided "as is"; IOGP disclaims all
warranties as to its accuracy and does not endorse CrsKit. The `tools/epsg-sqlite` utility builds the
SQLite database from the EPSG dataset SQL scripts; an unmodified build is published, for the test
suite, in the [`epsg-data`](https://github.com/digi21/crskit/releases/tag/epsg-data) release.

**Grid files** (geoid models, NTv2 datum-shift grids, …) are *not* redistributed here — they are
obtained by the user from their official sources (national mapping agencies), each under its own licence.
[`data/grid-sources.json`](data/grid-sources.json) maps a grid name to where it can be obtained, so an
application can offer to fetch it when the library reports one missing. The global **EGM2008** geoid
(NGA) is mirrored for convenience at `https://digi21.blob.core.windows.net/geodetic-grids/`, in the
little-endian layout the reader expects.

## License

Apache-2.0. See [LICENSE](LICENSE).
