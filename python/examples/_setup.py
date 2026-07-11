"""Shared start-up for the examples: initialise the library and locate the grid files.

Every example is a runnable script. They all work without grid files; the ones that need a grid say
which one, and pick it up automatically if you point CRSKIT_GRIDS at a folder holding them.
"""

import os

import crskit


def initialise() -> None:
    """crskit.init(), with the grid directory taken from CRSKIT_GRIDS when it is set."""
    crskit.init(data_directory=os.environ.get("CRSKIT_GRIDS"))
    print(f"EPSG {crskit.epsg_version()}")
