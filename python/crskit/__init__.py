"""CrsKit: coordinate reference systems and coordinate transformations.

An OGC Coordinate Transformation Services implementation backed by the EPSG Geodetic
Parameter Dataset, with WKT 1 (OGC and ESRI) and WKT 2 (2015 and 2019) support.

    >>> import crskit
    >>> crskit.init("epsg.sqlite")
    >>> etrs89 = crskit.crs_from_epsg(4258)          # geographic, lat/lon
    >>> utm30n = crskit.crs_from_epsg(25830)         # projected
    >>> crskit.transformation(etrs89, utm30n).transform([40.4, -3.7])

Inside an application that embeds Python and already uses CrsKit (the script window of
Digi3D.NET or MDTopX), the module binds to the library the application has already loaded and
shares its state: is_initialized() is then already True and init() need not be called.
"""

import os
import sys
from pathlib import Path

# Windows does not search the module's own folder for the DLLs it imports, and crskit.dll ships
# right next to _crskit.pyd inside this package. If the host process already has crskit.dll loaded
# (an application embedding Python), the loader reuses that one and this directory is not consulted.
if sys.platform == "win32":
    os.add_dll_directory(str(Path(__file__).parent))

from ._crskit import (
    AuthorityCodeNotFoundError,
    CoordinateOutsideDomainError,
    Crs,
    CrsError,
    DimensionMismatchError,
    GridFileNotFoundError,
    Transformation,
    TransformationNotFoundError,
    UnsupportedFormatError,
    WktParseError,
    WktVersion,
    crs_from_epsg,
    crs_from_wkt,
    epsg_version,
    init,
    is_initialized,
    transformation,
)

__all__ = [
    "AuthorityCodeNotFoundError",
    "CoordinateOutsideDomainError",
    "Crs",
    "CrsError",
    "DimensionMismatchError",
    "GridFileNotFoundError",
    "Transformation",
    "TransformationNotFoundError",
    "UnsupportedFormatError",
    "WktParseError",
    "WktVersion",
    "crs_from_epsg",
    "crs_from_wkt",
    "epsg_version",
    "init",
    "is_initialized",
    "transformation",
]
