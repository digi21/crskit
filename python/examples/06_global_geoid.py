"""The global geoid: EGM2008, anywhere on Earth.

Where a national geoid model exists it is the better one, but EGM2008 covers the whole planet. EPSG
offers it in two resolutions -- 2.5 arc-minutes (0.113 m) and 1 arc-minute (0.11 m) -- so it is again
a choice, and the sensible criterion is which grid you actually have on disk.

Note what comes back: a 3D point in, a 3D point out. The horizontal coordinates travel through
untouched and only the height changes, from ellipsoidal to orthometric.

Needs an EGM2008 grid; see the README for where to get it, and point CRSKIT_GRIDS at the folder.
"""

from pathlib import Path

from _setup import initialise

import crskit

initialise()

wgs84_3d = crskit.crs_from_epsg(4979)  # latitude, longitude, ellipsoidal height
egm2008 = crskit.crs_from_epsg(3855)  # EGM2008 height, the global orthometric height

print(f"source : {wgs84_3d.name} {wgs84_3d.axes}")
print(f"target : {egm2008.name} ({egm2008.dimension}D)\n")


chosen = None


def the_grid_i_have(source_name, target_name, operations):
    """Prefer the grid that is already in the data directory; fall back to the most accurate."""
    global chosen

    grids = grids_on_disk()
    for operation in sorted(operations, key=lambda o: o.accuracy or float("inf")):
        have = all(grid in grids for grid in operation.grid_files)
        print(f"  EPSG:{operation.code}  {operation.accuracy} m  {operation.grid_files[0]}"
              f"{'   <- on disk' if have else ''}")
        if have and chosen is None:
            chosen = operation

    return chosen or min(operations, key=lambda o: o.accuracy or float("inf"))


def grids_on_disk() -> set[str]:
    import os

    folder = os.environ.get("CRSKIT_GRIDS")
    return {path.name for path in Path(folder).iterdir()} if folder else set()


try:
    transformation = crskit.transformation(wgs84_3d, egm2008, select_operation=the_grid_i_have)
    print(f"\nusing EPSG:{chosen.code} {chosen.name} ({chosen.accuracy} m)\n")

    # Ellipsoidal heights, the kind a GNSS receiver reports.
    for place, latitude, longitude, ellipsoidal in [
        ("Madrid", 40.416775, -3.703790, 700.00),
        ("Everest", 27.988056, 86.925278, 8820.43),
        ("Sydney", -33.868820, 151.209290, 60.00),
    ]:
        latitude_out, longitude_out, orthometric = transformation.transform([latitude, longitude, ellipsoidal])
        undulation = ellipsoidal - orthometric
        print(f"  {place:<8} h={ellipsoidal:8.2f} m -> H={orthometric:8.2f} m   (geoid undulation {undulation:+7.2f} m)")

    print("\n(Everest's orthometric height comes out at the 8848.8 m everyone knows,"
          "\n from an ellipsoidal height 28 m lower: that is the geoid doing its job.)")

except crskit.GridFileNotFoundError as error:
    print(f"\nthis needs the global geoid grid, which is not here:")
    print(f"  file        : {error.grid_file}")
    print(f"  operation   : EPSG:{error.operation_code} {error.operation_name}")
    print(f"  published by: {error.information_source}")
    print("\nSee the README: download it, then set CRSKIT_GRIDS to the folder holding it.")
