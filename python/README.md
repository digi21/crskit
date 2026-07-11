# CrsKit for Python

Coordinate reference systems, Well-Known Text and coordinate transformations, backed by the full
EPSG catalogue. A thin binding over [CrsKit](https://github.com/digi21/crskit), a C++23
implementation of the OGC *Coordinate Transformation Services* model, conformance-tested against the
IOGP **GIGS** series.

```bash
pip install crskit
```

## Getting started

CrsKit is the *software*; the EPSG *data* are a separate download (see [The EPSG database](#the-epsg-database)
below). Point `init()` at it once, and you are ready:

```python
import crskit

crskit.init("epsg.sqlite")

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

Every error the library raises derives from `crskit.CrsError`, so one `except` catches them all;
`AuthorityCodeNotFoundError`, `WktParseError`, `TransformationNotFoundError`,
`DimensionMismatchError`, `CoordinateOutsideDomainError` and `UnsupportedFormatError` let you
discriminate when you care.

## The EPSG database

CrsKit does not bundle the EPSG Geodetic Parameter Dataset: it is IOGP's data, distributed under its
own [terms of use](https://epsg.org/terms-of-use.html). Get a prebuilt SQLite copy from the
[`epsg-data`](https://github.com/digi21/crskit/releases/tag/epsg-data) release, or build one from the
official EPSG SQL scripts with [`tools/epsg-sqlite`](https://github.com/digi21/crskit/tree/main/tools/epsg-sqlite).
Then hand it to `init()`.

## Inside Digi3D.NET and MDTopX

In the Python window of an application that embeds CrsKit, the module binds to the `crskit.dll` the
application already has loaded, and shares its state: the EPSG catalogue and the settings are the
ones the application set up, so there is nothing to initialise.

```python
if not crskit.is_initialized():
    crskit.init("epsg.sqlite")
```

## Typing

The package ships type stubs and a `py.typed` marker (PEP 561), so editors complete the API and
`mypy` checks it, even though the module is compiled C++.

## License

Apache-2.0. The EPSG dataset is IOGP's, used under its own terms; this product is not endorsed by
IOGP.
