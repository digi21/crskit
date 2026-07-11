"""The EPSG Geodetic Parameter Dataset, as a SQLite database, packaged for CrsKit.

The data are owned by the International Association of Oil & Gas Producers (IOGP) and are used
under the EPSG Terms of Use, a copy of which travels with this package (EPSG-TERMS-OF-USE.md) and
which you accept by using it. The database is a faithful format conversion of the SQL scripts IOGP
publishes -- EPSG's own schema, no parameter value altered. IOGP does not endorse CrsKit and
provides the dataset "as is", without warranty of any kind.

    >>> import crskit
    >>> crskit.init()          # finds this package on its own

    >>> import crskit_epsg
    >>> crskit_epsg.database_path()
    PosixPath('.../crskit_epsg/epsg.sqlite')
"""

from pathlib import Path

from ._version import EPSG_VERSION as EPSG_VERSION
from ._version import __version__ as __version__

__all__ = ["EPSG_VERSION", "database_path", "terms_of_use_path", "__version__"]


def database_path() -> Path:
    """The bundled EPSG SQLite database. Hand it to crskit.init()."""
    database = Path(__file__).parent / "epsg.sqlite"
    if not database.exists():
        raise FileNotFoundError(
            f"the EPSG database is missing from the package: {database}. The wheel is built by "
            "fetch.py, which puts it there; a source checkout has to run that first."
        )
    return database


def terms_of_use_path() -> Path:
    """The EPSG Terms of Use the data are distributed under. Read them: they bind you too."""
    return Path(__file__).parent / "EPSG-TERMS-OF-USE.md"
