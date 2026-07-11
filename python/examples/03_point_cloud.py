"""Transform a point cloud: one call, one crossing into C++, the GIL released.

transform() is fine for a point; for a million of them use transform_points(), which takes a NumPy
array and hands the whole buffer to the library at once. The difference is not subtle.
"""

import time

from _setup import initialise

import numpy as np

import crskit

initialise()

transformation = crskit.transformation(crskit.crs_from_epsg(4258), crskit.crs_from_epsg(25830))

# A million points scattered over the Iberian peninsula.
rng = np.random.default_rng(seed=42)
points = np.column_stack([rng.uniform(36.0, 43.0, 1_000_000), rng.uniform(-9.0, 3.0, 1_000_000)])

start = time.perf_counter()
projected = transformation.transform_points(points)
bulk = time.perf_counter() - start
print(f"transform_points: {len(points):,} points in {bulk:.3f} s -> array {projected.shape}")

start = time.perf_counter()
for point in points[:10_000]:
    transformation.transform(point)
per_point = (time.perf_counter() - start) * len(points) / 10_000
print(f"transform()     : the same would take about {per_point:.1f} s, one point at a time")

print(f"\nfirst point {points[0].round(6)} -> {projected[0].round(3)}")
