<p align="center">
  <img src="branding/crskit-logo-256.png" alt="CrsKit" width="120" height="120">
</p>

<h1 align="center">CrsKit</h1>

<p align="center">A modern C++23 geodesy library for coordinate reference systems, WKT and coordinate transformations.</p>

<p align="center">
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-00599C">
  <img alt="Platforms" src="https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-informational">
  <img alt="License" src="https://img.shields.io/badge/license-Apache--2.0-blue">
</p>

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
Geodetic Parameter Dataset). Point `DIGI21_EPSG_SQLITE` at a copy of it and run:

```bash
DIGI21_EPSG_SQLITE=/path/to/epsg.sqlite ctest --test-dir build --output-on-failure
```

The `tools/epsg-sqlite` utility builds the database from the EPSG dataset SQL scripts.

## Data and attribution

CrsKit is the **software**; coordinate reference data are supplied by the user. This product uses the
**EPSG Geodetic Parameter Dataset**, owned by IOGP, under the EPSG Dataset Terms of Use. Grid files
(geoid models, NTv2, …) are obtained by the user from their official sources, subject to each grid's
own licence.

## License

Apache-2.0. See [LICENSE](LICENSE).
