# CrsKit for Python — a tutorial

This is the long way in. It assumes you know Python and no geodesy at all: every idea is introduced
when you first need it, and every output below was produced by running the code.

If you only want the reference, the [README](README.md) is shorter. If you want runnable versions of
what follows, they are in [`examples/`](examples/).

---

## 1. Install it

```bash
pip install crskit[epsg]
```

Two packages arrive: `crskit`, the library, and `crskit-epsg`, the EPSG catalogue — the database of
every coordinate reference system the world has agreed to name. The catalogue is data owned by IOGP,
so it lives in a package of its own; that is also why it is versioned after the dataset rather than
after the library.

```python
import crskit

crskit.init()
print(crskit.epsg_version())     # 12.057
```

`init()` finds the catalogue on its own. Point it elsewhere if you keep your own copy:

```python
crskit.init("/path/to/epsg.sqlite", data_directory="/path/to/grids")
```

The `data_directory` is where *grid files* live. Ignore it for now — §6 explains when you need one.

---

## 2. What a coordinate reference system actually is

A pair of numbers like `(440291, 4474254)` means nothing on its own. It means something only once you
say **which system** the numbers are in: what shape the Earth is assumed to have, where its centre
is, and how the round surface was flattened onto a plane.

EPSG gives each of these systems a code. You will meet three kinds:

- **Geographic** — latitude and longitude in degrees, on an ellipsoid. `EPSG:4326` is WGS 84, the one
  your phone reports. `EPSG:4258` is ETRS89, the European one.
- **Projected** — metres on a plane, after a projection. `EPSG:25830` is ETRS89 / UTM zone 30N, which
  covers Spain.
- **Vertical** — heights, and only heights. `EPSG:5782` is Alicante height, the Spanish sea level.

Ask the library:

```python
utm30n = crskit.crs_from_epsg(25830)

utm30n.name             # 'ETRS89 / UTM zone 30N'
utm30n.authority_code   # 25830
utm30n.dimension        # 2
utm30n.axes             # [('E', 'East'), ('N', 'North')]
```

**That last line is the one that bites people.** A CRS declares the *order* of its coordinates, and
the order is not always the one you assume:

```python
>>> crskit.crs_from_epsg(4258).axes
[('Lat', 'North'), ('Lon', 'East')]
```

EPSG 4258 is *latitude first*. So is 4326. CrsKit takes coordinates in the order the system declares —
if you feed it longitude first, you will get a wrong answer, not an error, because your point is
simply somewhere else. When in doubt, print `.axes`.

---

## 3. Your first transformation

A **transformation** goes from one system to another. Build it once, use it many times.

```python
etrs89 = crskit.crs_from_epsg(4258)      # latitude, longitude
utm30n = crskit.crs_from_epsg(25830)     # easting, northing

transformation = crskit.transformation(etrs89, utm30n)

madrid = [40.416775, -3.703790]          # latitude, longitude — as 4258 declares
transformation.transform(madrid)
# [440291.2843479216, 4474254.600145094]
```

Going back is another transformation, built the other way round:

```python
crskit.transformation(utm30n, etrs89).transform([440291.284, 4474254.600])
# [40.41677499..., -3.70379000...]
```

A transformation knows what it is:

```python
transformation.source_dimension   # 2
transformation.target_dimension   # 2
transformation.is_identity        # False
transformation.source             # <Crs EPSG:4258 "ETRS89" (2D)>
```

---

## 4. Well-Known Text: how systems travel

A `.prj` file next to a shapefile, the `crs` field of a GeoTIFF, the string in a database column —
all of them carry a CRS as **Well-Known Text**. There are several dialects, and CrsKit reads all of
them, detecting which one it is from the text:

```python
crs = crskit.crs_from_wkt(open("layer.prj").read())
```

Writing is your choice of dialect:

```python
utm30n.to_wkt()                                # OGC WKT 1 — the default, most widely understood
utm30n.to_wkt(crskit.WktVersion.WKT1_ESRI)     # what ArcGIS expects in a .prj
utm30n.to_wkt(crskit.WktVersion.WKT2_2019)     # ISO 19162:2019, the modern one
```

```
WKT1        PROJCS["ETRS89 / UTM zone 30N",GEOGCS["ETRS89",DATUM["European T...
WKT1_ESRI   PROJCS["ETRS_1989_UTM_Zone_30N",GEOGCS["GCS_ETRS_1989",DATUM["D_...
WKT2_2019   PROJCRS["ETRS89 / UTM zone 30N",BASEGEOGCRS["ETRS89",DATUM["Euro...
```

### Comparing systems

Two systems are equal when they mean the same thing mathematically — same datum, same projection,
same parameters, same units. Names, authorities and axis order do not enter into it:

```python
>>> crskit.crs_from_wkt(utm30n.to_wkt()) == utm30n
True
```

That is `==` doing real work: the WKT round trip loses the EPSG code, and the systems still compare
equal because they *are* the same system. When you want to know *how* close two systems are, ask for
a score out of 100:

```python
>>> etrs89.compare(crskit.crs_from_epsg(4326))    # ETRS89 vs WGS 84
40
```

Forty, not a hundred: ETRS89 and WGS 84 are nearly the same today but are not the same system, and
the library will not pretend otherwise.

---

## 5. When there is more than one right answer

Here is where geodesy stops being a lookup table.

Moving between two **datums** — say the old European ED50 and the modern ETRS89 — is not a formula,
it is a measurement. Different agencies measured it differently, in different places, to different
accuracies. EPSG records **twelve** transformations from ED50 to ETRS89, and there is no way for a
library to know which one you want.

So CrsKit refuses to guess:

```python
>>> crskit.transformation(crskit.crs_from_epsg(4230), crskit.crs_from_epsg(4258))
TransformationNotFoundError: Multiple transformations were located between the 4230 (ED50)
coordinate system and the 4258 (ETRS89) coordinate system.
```

You choose, with a callback that receives the candidates:

```python
def choose(source_name, target_name, operations):
    for operation in sorted(operations, key=lambda o: o.accuracy or 99):
        print(f"EPSG:{operation.code}  {operation.accuracy} m  {operation.area_of_use}")
    ...

crskit.transformation(ed50, etrs89, select_operation=choose)
```

```
EPSG:15932   0.2 m  Spain - mainland and Balearic Islands onshore   (needs SPED2ETV2.gsb)
EPSG:1588    1.0 m  Norway - offshore north of 65°N; Svalbard
EPSG:1628    1.0 m  Gibraltar
EPSG:1630    1.5 m  Spain - Balearic Islands
EPSG:1632    1.5 m  Spain - mainland except northwest
EPSG:1634    1.5 m  Spain - mainland northwest
EPSG:1650    2.0 m  France
EPSG:1783    2.0 m  Turkey
EPSG:5040    5.0 m  Portugal - mainland - onshore
...
```

Read that list like a geographer, not like a programmer: **the smallest number is not the answer, the
right area is.** An operation valid in Norway will happily produce coordinates for a point in Spain,
and they will be wrong by hundreds of metres. Each candidate tells you what you need to decide:

```python
operation.code                # 1632
operation.accuracy            # 1.5   (metres)
operation.area_of_use         # 'Spain - mainland except northwest'
operation.grid_files          # []    — or the grid it needs, see §6
operation.information_source  # who published it
```

Return the operation you want (or just its code), and you get the transformation:

```python
def for_mainland_spain(source_name, target_name, operations):
    spain = [o for o in operations if "Spain - mainland" in o.area_of_use and not o.grid_files]
    return min(spain, key=lambda o: o.accuracy)

transformation = crskit.transformation(ed50, etrs89, select_operation=for_mainland_spain)
transformation.transform([40.417659, -3.702557])
# [40.416478, -3.703862]     — about 130 m away: that is what a datum change looks like
```

---

## 6. Heights, geoids and grid files

Your GNSS receiver reports an **ellipsoidal height**: the distance to a mathematical ellipsoid. Nobody
means that by "altitude". The height on a map is **orthometric**: measured from the geoid, the surface
mean sea level would follow. The difference between the two — the *undulation* — is about +52 m over
Madrid and −28 m over Everest, and it varies from place to place, which is why it comes as a **grid
file**: a geoid model.

Grid files belong to the agencies that publish them, so no library ships them. CrsKit tells you
exactly which one it wants:

```python
try:
    crskit.transformation(source, target)
except crskit.GridFileNotFoundError as error:
    error.grid_file           # 'EGM08_REDNAP.txt'
    error.operation_code      # 9410
    error.information_source  # 'National Geographic Institute of Spain (IGN)...'
    error.searched_path       # where it looked
```

Download it (the [README](README.md#where-to-get-the-grids) lists the usual ones), drop it in a
folder, and pass that folder to `init(data_directory=...)`.

### Two ways to ask for a height

**A compound CRS** pairs a horizontal system with a vertical one. EPSG has no code for most
combinations, so you build it from its two parts:

```python
etrs89_3d = crskit.crs_from_epsg(4937)                  # latitude, longitude, ellipsoidal height
utm_msl = crskit.compound_crs(25830, 5782)              # UTM 30N + Alicante height

crskit.transformation(etrs89_3d, utm_msl).transform([40.416775, -3.703790, 700.0])
# [440291.284, 4474254.600, 648.888]
```

700 m over the ellipsoid is 648.888 m over the sea: the geoid there is 51.112 m up.

**Or transform straight to a vertical CRS**, when the height is all you care about. The point comes
back with its horizontal coordinates untouched and only the height converted:

```python
to_egm2008 = crskit.transformation(
    crskit.crs_from_epsg(4979),                          # WGS 84 3D
    crskit.crs_from_epsg(3855),                          # EGM2008 height — the global geoid
    select_operation=lambda source, target, ops: ops[0])

to_egm2008.transform([27.988056, 86.925278, 8820.43])    # Everest, ellipsoidal
# [27.988056, 86.925278, 8848.86]
```

That last number is Everest's height as everyone knows it, and it fell out of an ellipsoidal height
28 m lower. If you want one check that the whole stack is working, that is it.

---

## 7. Many points at once

`transform()` crosses from Python into C++ once per point. For a point cloud, hand over the whole
buffer instead: `transform_points()` takes a NumPy array of shape `(n, dimensions)`, releases the GIL,
and comes back with an array.

```python
import numpy as np

points = np.column_stack([latitudes, longitudes])        # (1_000_000, 2)
projected = transformation.transform_points(points)      # (1_000_000, 2)
```

```
transform_points: 1,000,000 points in 0.394 s
transform()     : the same, one point at a time, would take about 2.2 s
```

Five or six times faster, and the gap widens with the point count. NumPy is optional — `transform()`
works with plain lists, and the module imports without NumPy installed.

---

## 8. When things go wrong

Every error the library raises descends from `crskit.CrsError`, so one `except` catches the lot. The
specific types are there for when you can actually do something about it:

| | |
|---|---|
| `AuthorityCodeNotFoundError` | there is no such EPSG code |
| `WktParseError` | the WKT is malformed |
| `TransformationNotFoundError` | no operation between those systems — or several, and you did not choose (§5) |
| `GridFileNotFoundError` | a grid file is missing; the exception says which (§6) |
| `CoordinateOutsideDomainError` | the point is outside the grid or the projection's valid area |
| `DimensionMismatchError` | you gave a 2D point to a 3D transformation, or the reverse |
| `UnsupportedFormatError` | the CRS kind or operation method is not supported |

`CoordinateOutsideDomainError` deserves a word. It usually means what it says — the point is outside
the area the grid covers — but it is also what you get when you feed coordinates in the wrong axis
order, because the transposed point lands in the sea. If a transformation that ought to work reports
a point outside its domain, check `.axes` before anything else.

---

## 9. Inside an application that embeds Python

If you are running scripts inside an application that already uses CrsKit, the module binds to the
library the application has loaded and shares its state — its EPSG catalogue, its settings. There is
nothing to initialise:

```python
if not crskit.is_initialized():
    crskit.init()
```

---

## Where to go next

- [`examples/`](examples/) — the runnable versions of everything above.
- [README](README.md) — the reference, and where to download the grid files.
- [The C++ library](https://github.com/digi21/crskit) — this binding is a thin layer over it, and the
  C++ API is where anything not exposed here can be found.
