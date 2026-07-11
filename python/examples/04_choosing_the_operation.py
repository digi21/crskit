"""When EPSG offers several ways to get there, you choose -- the library will not guess.

Between two datums there is rarely a single transformation. EPSG defines twelve from ED50 to ETRS89:
they differ in accuracy (from 0.2 m to 5 m), in the area they are valid in (Spain, Norway, Turkey...)
and in whether they need a grid file. Picking one for you would mean silently deciding how wrong your
coordinates are, so an ambiguous pair raises TransformationNotFoundError unless you pass
select_operation.

The callback receives the candidates and returns the one to use.
"""

from _setup import initialise

import crskit

initialise()

ed50 = crskit.crs_from_epsg(4230)
etrs89 = crskit.crs_from_epsg(4258)

# Without a choice, the library refuses rather than guess.
try:
    crskit.transformation(ed50, etrs89)
except crskit.TransformationNotFoundError as error:
    print(f"ambiguous: {error}\n")


def for_mainland_spain(source_name, target_name, operations):
    """Pick the most accurate operation valid in mainland Spain that needs no grid we lack."""
    print(f"{len(operations)} candidate operations from {source_name} to {target_name}:")
    for operation in sorted(operations, key=lambda o: o.accuracy or float("inf")):
        grids = f" needs {', '.join(operation.grid_files)}" if operation.grid_files else ""
        print(f"  EPSG:{operation.code:<6} {operation.accuracy or '?':>4} m  {operation.area_of_use}{grids}")

    spain = [o for o in operations if "Spain - mainland" in o.area_of_use and not o.grid_files]
    return min(spain, key=lambda o: o.accuracy or float("inf"))


transformation = crskit.transformation(ed50, etrs89, select_operation=for_mainland_spain)
print(f"\nchosen: EPSG:{transformation.authority_code} {transformation.name}")
print(f"        valid in {transformation.area_of_use}")

madrid_ed50 = [40.417659, -3.702557]
print(f"\nMadrid in ED50 {madrid_ed50} -> ETRS89 {[round(c, 6) for c in transformation.transform(madrid_ed50)]}")

# The most accurate operation of all (0.2 m) needs a grid published by the Spanish IGN. Ask for it
# and, if the grid is not in the data directory, the error says exactly which file to obtain.
try:
    crskit.transformation(ed50, etrs89, select_operation=lambda s, t, ops: min(ops, key=lambda o: o.accuracy or 9e9))
except crskit.GridFileNotFoundError as error:
    print(f"\nthe 0.2 m operation needs a grid that is not here:")
    print(f"  file        : {error.grid_file}")
    print(f"  operation   : EPSG:{error.operation_code} ({error.area_of_use})")
    print(f"  published by: {error.information_source}")
    print(f"  looked in   : {error.searched_path}")
