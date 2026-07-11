"""CrsKit: coordinate reference systems and coordinate transformations.

An OGC Coordinate Transformation Services implementation backed by the EPSG Geodetic
Parameter Dataset, with WKT 1 (OGC and ESRI) and WKT 2 (2015 and 2019) support.

    >>> import crskit
    >>> crskit.init("epsg.sqlite")
    >>> etrs89 = crskit.crs_from_epsg(4258)          # geographic, lat/lon
    >>> utm30n = crskit.crs_from_epsg(25830)         # projected
    >>> crskit.transformation(etrs89, utm30n).transform([40.4, -3.7])
"""

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
    "transformation",
]
