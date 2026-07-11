"""CrsKit: coordinate reference systems and coordinate transformations.

An OGC Coordinate Transformation Services implementation backed by the EPSG Geodetic
Parameter Dataset, with WKT 1 (OGC and ESRI) and WKT 2 (2015 and 2019) support.

    >>> import crskit
    >>> crskit.init()                                # with crskit-epsg installed
    >>> etrs89 = crskit.crs_from_epsg(4258)          # geographic, lat/lon
    >>> utm30n = crskit.crs_from_epsg(25830)         # projected
    >>> crskit.transformation(etrs89, utm30n).transform([40.4, -3.7])

Inside a host application that embeds Python and already uses CrsKit, the module binds to the
library the application has already loaded and shares its state: is_initialized() is then already
True and init() need not be called.
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
    CoordinateOperation,
    CoordinateOutsideDomainError,
    Crs,
    CrsError,
    DimensionMismatchError,
    GridFileNotFoundError,
    Transformation,
    TransformationNotFoundError,
    UnknownCrsPolicy,
    UnsupportedFormatError,
    WktParseError,
    WktVersion,
    compound_crs,
    crs_from_epsg,
    crs_from_wkt,
    epsg_version,
    is_initialized,
    transformation,
)
from ._crskit import init as _init


def _find_epsg_database() -> Path:
    """The EPSG database to use when init() is given none: the environment, then the data package."""
    if environment := os.environ.get("DIGI21_EPSG_SQLITE"):
        return Path(environment)

    try:
        import crskit_epsg
    except ImportError:
        raise CrsError(
            "no EPSG database: install one with `pip install crskit[epsg]`, set DIGI21_EPSG_SQLITE, "
            "or pass the path to init()."
        ) from None

    return crskit_epsg.database_path()


def init(
    epsg_database: os.PathLike[str] | str | None = None,
    data_directory: os.PathLike[str] | str | None = None,
) -> None:
    """Initialises the library against an EPSG SQLite database.

    With no arguments the database is looked up in the DIGI21_EPSG_SQLITE environment variable and
    then in the crskit-epsg package, so `pip install crskit[epsg]` needs no path at all.

    data_directory is where grid files (geoids, NTv2, NADCON) are looked up; it defaults to the
    folder holding the database.
    """
    _init(epsg_database if epsg_database is not None else _find_epsg_database(), data_directory)

__all__ = [
    "AuthorityCodeNotFoundError",
    "CoordinateOperation",
    "CoordinateOutsideDomainError",
    "Crs",
    "CrsError",
    "DimensionMismatchError",
    "GridFileNotFoundError",
    "Transformation",
    "TransformationNotFoundError",
    "UnknownCrsPolicy",
    "UnsupportedFormatError",
    "WktParseError",
    "WktVersion",
    "compound_crs",
    "crs_from_epsg",
    "crs_from_wkt",
    "epsg_version",
    "init",
    "is_initialized",
    "transformation",
]
