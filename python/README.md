# CrsKit for Python

Coordinate reference systems, Well-Known Text and coordinate transformations, backed by the full
EPSG catalogue. A thin binding over [CrsKit](https://github.com/digi21/crskit), a C++23
implementation of the OGC *Coordinate Transformation Services* model, conformance-tested against the
IOGP **GIGS** series.

```bash
pip install crskit[epsg]
```

## Getting started

The `[epsg]` extra brings the EPSG catalogue with it, so `init()` needs no path — it finds the
database on its own:

```python
import crskit

crskit.init()

etrs89 = crskit.crs_from_epsg(4258)     # ETRS89, geographic 2D (latitude, longitude)
utm30n = crskit.crs_from_epsg(25830)    # ETRS89 / UTM zone 30N (easting, northing)

madrid = crskit.transformation(etrs89, utm30n).transform([40.416775, -3.703790])
print(madrid)                           # [440291.2843479216, 4474254.600145094]
```

Coordinates are given in the axis order the CRS declares — EPSG 4258 is *latitude, longitude*, not
the other way round. Ask the system if you are unsure:

```python
>>> etrs89.axes
[('Lat', 'North'), ('Lon', 'East')]
>>> utm30n.axes
[('E', 'East'), ('N', 'North')]
```

## Transforming many points

`transform_points()` takes a NumPy array of shape `(n, source_dimension)` and crosses into C++ once
for the whole batch, releasing the GIL while it works — the path to use for point clouds:

```python
import numpy as np

points = np.array([[40.416775, -3.703790],      # Madrid
                   [41.385064,  2.173404],      # Barcelona
                   [37.389092, -5.984459]])     # Sevilla

transformation = crskit.transformation(etrs89, utm30n)
projected = transformation.transform_points(points)      # -> (3, 2) array
```

NumPy is optional: `transform()` works with any sequence of floats, and the module imports without it.

## Coordinate reference systems

Build a CRS from an EPSG code or from Well-Known Text (WKT 1 and WKT 2 are both understood; the
grammar is detected from the text):

```python
crs = crskit.crs_from_wkt(open("layer.prj").read())

crs.name             # 'ETRS89 / UTM zone 30N'
crs.authority_code   # 25830, or None when the WKT carries no authority
crs.dimension        # 2
crs.remarks
```

Emit WKT in any of the flavours the library supports:

```python
crs.to_wkt()                                        # OGC WKT 1 (the default)
crs.to_wkt(crskit.WktVersion.WKT2_2019)             # ISO 19162:2019
crs.to_wkt(crskit.WktVersion.WKT1_ESRI)             # ESRI dialect, for a .prj ArcGIS will read
```

Two systems compare by their **mathematical definition**, not by their name or authority: `==` asks
"are these the same CRS for transformation purposes?", ignoring naming and axis order.

```python
>>> crskit.crs_from_wkt(utm30n.to_wkt()) == utm30n
True
>>> etrs89.compare(utm30n)      # graded similarity, 0..100
0
```

## Choosing between coordinate operations

Between two datums there is rarely a single transformation. EPSG defines **twelve** from ED50 to
ETRS89, differing in accuracy, in the area they are valid in, and in whether they need a grid file.
Choosing for you would mean silently deciding how wrong your coordinates are, so an ambiguous pair
raises `TransformationNotFoundError` unless you say which one to use:

```python
def for_mainland_spain(source_name, target_name, operations):
    for operation in operations:
        print(f"EPSG:{operation.code}  {operation.accuracy} m  {operation.area_of_use}")
    spain = [o for o in operations if "Spain - mainland" in o.area_of_use and not o.grid_files]
    return min(spain, key=lambda o: o.accuracy)

transformation = crskit.transformation(ed50, etrs89, select_operation=for_mainland_spain)
```

```
EPSG:15932   0.2 m  Spain - mainland and Balearic Islands onshore   needs SPED2ETV2.gsb
EPSG:1588    1.0 m  Norway - offshore north of 65°N; Svalbard
EPSG:1628    1.0 m  Gibraltar
EPSG:1632    1.5 m  Spain - mainland except northwest
EPSG:1650    2.0 m  France
EPSG:5040    5.0 m  Portugal - mainland - onshore
...
```

Each candidate tells you its `code`, `accuracy` in metres, `area_of_use`, `grid_files` and
`information_source` — enough to choose by geography and not only by the smallest number.

## Heights above sea level

A GNSS receiver gives an *ellipsoidal* height, which is not the altitude anyone means. Turning it
into an orthometric height takes a geoid model. Pair a horizontal system with a vertical one and
transform in 3D:

```python
etrs89_3d = crskit.crs_from_epsg(4937)              # latitude, longitude, ellipsoidal height
utm_over_sea_level = crskit.compound_crs(25830, 5782)   # UTM 30N + Alicante height

crskit.transformation(etrs89_3d, utm_over_sea_level).transform([40.416775, -3.703790, 700.0])
# [440291.284, 4474254.600, 648.888]   <- 51.112 m of geoid undulation over Madrid
```

Or transform straight to a vertical CRS, and get the point back with only its height changed —
EGM2008 (EPSG 3855) is the global model, so this works anywhere:

```python
to_egm2008 = crskit.transformation(crskit.crs_from_epsg(4979), crskit.crs_from_epsg(3855),
                                   select_operation=lambda source, target, ops: ops[0])

to_egm2008.transform([27.988056, 86.925278, 8820.43])   # Everest, ellipsoidal height
# [27.988056, 86.925278, 8848.86]   <- the orthometric height everyone knows
```

## When a transformation needs a grid

Datum shifts and orthometric heights often need a grid file (a geoid model, an NTv2 grid) that
cannot be redistributed with the software. When one is missing, the error tells you exactly which
file to obtain and for which EPSG operation:

```python
try:
    crskit.transformation(source, target)
except crskit.GridFileNotFoundError as error:
    print(error.grid_file)           # 'egm08_25.gtx'
    print(error.searched_path)       # where it was looked for
    print(error.operation_code)      # the EPSG coordinate operation that needs it
    print(error.information_source)  # who publishes it
```

Put the grids in the data directory (`init(..., data_directory=...)`, which defaults to the folder
holding the EPSG database) and the transformation builds.

### Where to get the grids

Grid files belong to the agencies that publish them, so they are not shipped with CrsKit. The two
you are most likely to need:

**EGM2008**, the global geoid, published by the NGA — save the file under the name EPSG gives it,
which is the name in the link:

- [`Und_min2.5x2.5_egm2008_isw=82_WGS84_TideFree`](https://digi21.blob.core.windows.net/geodetic-grids/Und_min2.5x2.5_egm2008_isw%3D82_WGS84_TideFree) (2.5′, 149 MB) — enough for most work.
- [`Und_min1x1_egm2008_isw=82_WGS84_TideFree`](https://digi21.blob.core.windows.net/geodetic-grids/Und_min1x1_egm2008_isw%3D82_WGS84_TideFree) (1′, 933 MB) — the finer grid.

These are mirrors of the NGA files, in the little-endian layout the reader expects; the originals are
at [earth-info.nga.mil](https://earth-info.nga.mil/index.php?dir=wgs84&action=wgs84).

**Spain** — the EGM08-REDNAP geoid, which is the model EPSG uses for heights over Alicante datum:

- [`EGM08_REDNAP.txt`](https://digi21.blob.core.windows.net/geodetic-grids/EGM08_REDNAP.txt) (4 MB) — peninsula and Balearics.
- [`EGM08_REDNAP_Canarias.txt`](https://digi21.blob.core.windows.net/geodetic-grids/EGM08_REDNAP_Canarias.txt) (0.4 MB) — the Canary Islands.

The ED50↔ETRS89 NTv2 grids (`PENR2009.gsb`, `BALR2009.gsb`) come from the
[CNIG download centre](https://centrodedescargas.cnig.es/CentroDescargas/redes-geodesicas). All of
these are © *Instituto Geográfico Nacional de España*, mirrored under its CC BY 4.0-compatible
licence; the originals live at [datos-geodesia.ign.es](https://datos-geodesia.ign.es/geoide/).

`data/grid-sources.json` in the repository maps a grid name to where it can be obtained, so an
application can offer to fetch it when `GridFileNotFoundError` is raised.

Every error the library raises derives from `crskit.CrsError`, so one `except` catches them all;
`AuthorityCodeNotFoundError`, `WktParseError`, `TransformationNotFoundError`,
`DimensionMismatchError`, `CoordinateOutsideDomainError` and `UnsupportedFormatError` let you
discriminate when you care.

## The EPSG database

The catalogue lives in its own package, [`crskit-epsg`](https://pypi.org/project/crskit-epsg/),
which the `[epsg]` extra installs. It is versioned after the dataset, not after the library, so a
newer EPSG release is `pip install -U crskit-epsg` — no new version of CrsKit needed.

`init()` looks for the database in this order: the path you pass, the `DIGI21_EPSG_SQLITE`
environment variable, and then the `crskit-epsg` package. Any SQLite built from the official EPSG
SQL scripts works — for instance one you build yourself with
[`tools/epsg-sqlite`](https://github.com/digi21/crskit/tree/main/tools/epsg-sqlite) for a version
newer than the packaged one:

```python
crskit.init("/path/to/epsg.sqlite", data_directory="/path/to/grids")
```

The data are the EPSG Geodetic Parameter Dataset (<https://epsg.org>), **owned by IOGP** and used
under its [Terms of Use](https://epsg.org/terms-of-use.html), which ship inside `crskit-epsg`. IOGP
provides them "as is" and does not endorse CrsKit.

## Inside an application that embeds Python

The module links the CrsKit shared library rather than compiling it in. So when a host application
that already uses CrsKit runs a script through an embedded interpreter, the loader binds the module
to the library the application has *already* loaded, and both share its state: the EPSG catalogue
and settings are the ones the application set up, and there is nothing left to initialise.

```python
if not crskit.is_initialized():
    crskit.init()
```

## Learning it

- **[TUTORIAL.md](TUTORIAL.md)** — the long way in, from "what is a CRS" to geoids and point clouds.
  Written for someone who knows Python and no geodesy; every output in it was produced by running the
  code. Also in [español](TUTORIAL.es.md), [français](TUTORIAL.fr.md), [italiano](TUTORIAL.it.md) and
  [deutsch](TUTORIAL.de.md).
- **[`examples/`](examples/)** — runnable scripts: a quick start, WKT round trips, a million points
  with NumPy, choosing among candidate operations, and orthometric heights. The test suite runs them,
  so they cannot rot.

## Typing

The package ships type stubs and a `py.typed` marker (PEP 561), so editors complete the API and
`mypy` checks it, even though the module is compiled C++.

## License

Apache-2.0. The EPSG dataset is IOGP's, used under its own terms; this product is not endorsed by
IOGP.
