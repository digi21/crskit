# Examples

Runnable scripts, each one self-contained. They are part of the test suite, so they cannot rot.

```bash
pip install crskit[epsg] numpy
python 01_quickstart.py
```

| | |
|---|---|
| [`01_quickstart.py`](01_quickstart.py) | Get two systems by EPSG code and transform a point between them. |
| [`02_wkt.py`](02_wkt.py) | Emit WKT 1, ESRI and WKT 2; parse it back; compare systems by what they mean. |
| [`03_point_cloud.py`](03_point_cloud.py) | A million points in one call, with NumPy. |
| [`04_choosing_the_operation.py`](04_choosing_the_operation.py) | EPSG offers twelve ways from ED50 to ETRS89 — choose one. |
| [`05_orthometric_heights.py`](05_orthometric_heights.py) | Ellipsoidal height to height above sea level, through a geoid model. |

## Grid files

Some coordinate operations need a grid file — a geoid model, an NTv2 datum-shift grid. Those files
belong to the agencies that publish them and are not redistributed with CrsKit, so the examples run
without them: instead of failing, they print which file the operation wants and who publishes it.

To run `05_orthometric_heights.py` for real you need a geoid. Either the **global EGM2008**, from the
NGA (mirrored [here](https://digi21.blob.core.windows.net/geodetic-grids/Und_min2.5x2.5_egm2008_isw%3D82_WGS84_TideFree),
2.5′, 149 MB — save it under that same name), or, in Spain, the **EGM08-REDNAP** geoid from
[datos-geodesia.ign.es](https://datos-geodesia.ign.es/geoide/) (`EGM08_REDNAP.txt`), which is what
EPSG picks for Alicante height. `04_choosing_the_operation.py` mentions `SPED2ETV2.gsb`, an
ED50→ETRS89 grid from the [CNIG](https://centrodedescargas.cnig.es/CentroDescargas/redes-geodesicas).

Put them in a folder and point `CRSKIT_GRIDS` at it:

```bash
CRSKIT_GRIDS=/path/to/grids python 05_orthometric_heights.py
```
