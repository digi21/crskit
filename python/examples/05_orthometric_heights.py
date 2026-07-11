"""Heights above sea level: a compound CRS and a geoid model.

A GNSS receiver gives you an ellipsoidal height -- a height above a mathematical ellipsoid, which is
not the height anyone means by "altitude". Converting it to an orthometric height (above mean sea
level) takes a geoid model, and that is a grid file: a national one where it exists, EGM2008
otherwise.

A compound CRS pairs a horizontal system with a vertical one: compound_crs(25830, 5782) is UTM 30N
with heights over Alicante datum, the Spanish mean sea level.

This example needs a geoid grid. Point CRSKIT_GRIDS at the folder holding it; without it, the error
tells you which file to fetch, and from whom.
"""

from _setup import initialise

import crskit

initialise()

etrs89_3d = crskit.crs_from_epsg(4937)  # latitude, longitude, ellipsoidal height
utm_over_sea_level = crskit.compound_crs(25830, 5782)  # UTM 30N + Alicante height

print(f"source : {etrs89_3d.name} {etrs89_3d.axes}")
print(f"target : {utm_over_sea_level.name} ({utm_over_sea_level.dimension}D)")

madrid = [40.416775, -3.703790, 700.0]  # 700 m above the ellipsoid

try:
    transformation = crskit.transformation(etrs89_3d, utm_over_sea_level)
    easting, northing, height = transformation.transform(madrid)

    print(f"\nellipsoidal height {madrid[2]:.3f} m")
    print(f"orthometric height {height:.3f} m   (geoid undulation {madrid[2] - height:.3f} m)")
    print(f"projected          E {easting:.3f}  N {northing:.3f}")
except crskit.GridFileNotFoundError as error:
    print(f"\nthis needs a geoid model, which is not here:")
    print(f"  file        : {error.grid_file}")
    print(f"  operation   : EPSG:{error.operation_code} {error.operation_name}")
    print(f"  published by: {error.information_source}")
    print(f"\nObtain it, put it in a folder, and set CRSKIT_GRIDS to that folder.")
