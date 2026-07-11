"""Transform a point between two coordinate reference systems.

The essentials: get the systems by EPSG code, build the transformation, apply it. Note that the
coordinate order is the one the system declares -- EPSG 4258 is latitude, longitude.
"""

from _setup import initialise

import crskit

initialise()

etrs89 = crskit.crs_from_epsg(4258)  # ETRS89, geographic 2D
utm30n = crskit.crs_from_epsg(25830)  # ETRS89 / UTM zone 30N

print(f"source : {etrs89.name} {etrs89.axes}")
print(f"target : {utm30n.name} {utm30n.axes}")

transformation = crskit.transformation(etrs89, utm30n)

madrid = [40.416775, -3.703790]  # latitude, longitude
easting, northing = transformation.transform(madrid)
print(f"\nMadrid {madrid} -> E {easting:.3f}  N {northing:.3f}")

# Transformations run either way: build the one you need.
back = crskit.transformation(utm30n, etrs89).transform([easting, northing])
print(f"and back        -> {back[0]:.6f}, {back[1]:.6f}")
