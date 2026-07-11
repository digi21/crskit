"""Type stubs for the compiled module.

Generated with `python -m pybind11_stubgen crskit._crskit` and then corrected by hand, because the
generator cannot see through the C++ signatures: it types anything that returns py::object as Any
(authority_code is int | None, not Any) and the axis list as a bare list.
"""

import os
from collections.abc import Callable, Sequence
from typing import Any, ClassVar, SupportsFloat, SupportsIndex, SupportsInt

import numpy
import numpy.typing

__all__: list[str]

SelectOperation = Callable[[str, str, list["CoordinateOperation"]], "CoordinateOperation | int"]

class CrsError(RuntimeError):
    """Base of every error the library raises."""

class AuthorityCodeNotFoundError(CrsError): ...
class CoordinateOutsideDomainError(CrsError): ...
class DimensionMismatchError(CrsError): ...
class TransformationNotFoundError(CrsError): ...
class UnsupportedFormatError(CrsError): ...
class WktParseError(CrsError): ...

class GridFileNotFoundError(CrsError):
    """A grid file (geoid, NTv2, NADCON) a coordinate operation needs is not in the data directory."""

    grid_file: str
    searched_path: str
    operation_code: int | None
    operation_name: str
    area_of_use: str
    information_source: str

class WktVersion:
    """The Well-Known Text flavour to_wkt() emits."""

    WKT1: ClassVar[WktVersion]
    """OGC 01-009 (Simple Features)."""
    WKT1_ESRI: ClassVar[WktVersion]
    """ESRI WKT 1, without the EPSG authority code."""
    WKT1_ESRI_WITH_EPSG_CODE: ClassVar[WktVersion]
    """ESRI WKT 1, with the EPSG authority code."""
    WKT2_2015: ClassVar[WktVersion]
    """ISO 19162:2015 / OGC 12-063r5."""
    WKT2_2019: ClassVar[WktVersion]
    """ISO 19162:2019 / OGC 18-010r7."""

    __members__: ClassVar[dict[str, WktVersion]]

    def __init__(self, value: SupportsInt | SupportsIndex) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class UnknownCrsPolicy:
    """What to do when exactly one of the two systems is local or unknown."""

    REJECT: ClassVar[UnknownCrsPolicy]
    IDENTITY: ClassVar[UnknownCrsPolicy]

    __members__: ClassVar[dict[str, UnknownCrsPolicy]]

    def __init__(self, value: SupportsInt | SupportsIndex) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class CoordinateOperation:
    """An EPSG coordinate operation: one of the ways to get from one system to another."""

    @property
    def code(self) -> int: ...
    @property
    def name(self) -> str: ...
    @property
    def accuracy(self) -> float | None:
        """The accuracy in metres, or None when EPSG does not state one."""

    @property
    def area_of_use(self) -> str:
        """The geographic extent the operation is valid in."""

    @property
    def grid_files(self) -> list[str]:
        """The grid file(s) the operation needs, empty when it needs none."""

    @property
    def scope(self) -> str | None: ...
    @property
    def remarks(self) -> str | None: ...
    @property
    def information_source(self) -> str | None: ...
    def __repr__(self) -> str: ...

class Crs:
    """A coordinate reference system. Build one with crs_from_epsg() or crs_from_wkt()."""

    __hash__: ClassVar[None]  # structural equality has no cheap, consistent hash

    @property
    def name(self) -> str: ...
    @property
    def authority(self) -> str: ...
    @property
    def authority_code(self) -> int | None:
        """The authority code (the EPSG code), or None when the system carries none."""

    @property
    def dimension(self) -> int: ...
    @property
    def remarks(self) -> str: ...
    @property
    def axes(self) -> list[tuple[str, str]]:
        """The axes, as (name, orientation) pairs in axis order."""

    def to_wkt(self, version: WktVersion = ...) -> str:
        """The system's Well-Known Text in the requested flavour."""

    def compare(self, other: Crs) -> int:
        """Graded structural similarity with another system, 0..100 (100 == equivalent)."""

    def __eq__(self, other: object) -> bool:
        """Structural equivalence: same mathematical definition, ignoring name, authority and axis order."""

    def __repr__(self) -> str: ...

class Transformation:
    """A coordinate transformation between two systems. Build one with transformation()."""

    @property
    def name(self) -> str: ...
    @property
    def authority(self) -> str: ...
    @property
    def authority_code(self) -> str | None:
        """The EPSG code of the coordinate operation, or None when it is not one."""

    @property
    def area_of_use(self) -> str: ...
    @property
    def remarks(self) -> str: ...
    @property
    def source(self) -> Crs: ...
    @property
    def target(self) -> Crs: ...
    @property
    def source_dimension(self) -> int: ...
    @property
    def target_dimension(self) -> int: ...
    @property
    def is_identity(self) -> bool: ...
    def transform(self, point: Sequence[SupportsFloat | SupportsIndex]) -> list[float]:
        """Transforms a single point, given as a sequence of coordinates."""

    def transform_points(self, points: numpy.typing.ArrayLike) -> numpy.typing.NDArray[numpy.float64]:
        """Transforms a NumPy array of points, of shape (source_dimension,) or (n, source_dimension)."""

    def __repr__(self) -> str: ...

def init(
    epsg_database: os.PathLike[str] | str,
    data_directory: os.PathLike[str] | str | None = None,
) -> None:
    """Initialises the library against an EPSG SQLite database."""

def is_initialized() -> bool:
    """Whether the library already has an EPSG database (the host application may have set it)."""

def epsg_version() -> str:
    """The EPSG Geodetic Parameter Dataset version of the active database (e.g. '12.057')."""

def crs_from_epsg(code: SupportsInt | SupportsIndex) -> Crs:
    """The coordinate reference system with this EPSG code."""

def crs_from_wkt(wkt: str) -> Crs:
    """The coordinate reference system described by this Well-Known Text (WKT 1 or WKT 2)."""

def compound_crs(horizontal: SupportsInt | SupportsIndex, vertical: SupportsInt | SupportsIndex) -> Crs:
    """A compound CRS from the EPSG codes of a horizontal and a vertical system."""

def transformation(
    source: Crs,
    target: Crs,
    *,
    select_operation: SelectOperation | None = None,
    unknown_crs_policy: UnknownCrsPolicy = ...,
) -> Transformation:
    """The transformation from one coordinate reference system to another.

    When EPSG defines several operations between the two systems, select_operation chooses; without
    it, an ambiguous pair raises TransformationNotFoundError.
    """
