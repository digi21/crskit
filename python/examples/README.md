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

Once you have the grids, put them in a folder and point `CRSKIT_GRIDS` at it:

```bash
CRSKIT_GRIDS=/path/to/grids python 05_orthometric_heights.py
```
