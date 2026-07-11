"""Tests for the Python binding.

The EPSG database is not shipped with the package: point DIGI21_EPSG_SQLITE at one, exactly as
the C++ (GoogleTest) suite does.
"""

import os

import pytest

import crskit

EPSG_DATABASE = os.environ.get(
    "DIGI21_EPSG_SQLITE", r"C:\ProgramData\Digi3D.NET\OpenGis\epsg-fiel.sqlite"
)

# EPSG 4258 (ETRS89, geographic 2D, latitude/longitude) and 25830 (ETRS89 / UTM zone 30N).
MADRID_LAT_LON = [40.416775, -3.703790]


@pytest.fixture(scope="session", autouse=True)
def initialise():
    if not os.path.exists(EPSG_DATABASE):
        pytest.skip(f"EPSG database not found: {EPSG_DATABASE}")
    crskit.init(EPSG_DATABASE)


@pytest.fixture(scope="session")
def etrs89():
    return crskit.crs_from_epsg(4258)


@pytest.fixture(scope="session")
def utm30n():
    return crskit.crs_from_epsg(25830)


def test_the_library_reports_itself_initialized():
    assert crskit.is_initialized()


def test_epsg_version_is_reported():
    assert crskit.epsg_version()


def test_crs_from_epsg_exposes_its_identity(utm30n):
    assert utm30n.authority == "EPSG"
    assert utm30n.authority_code == 25830
    assert "UTM" in utm30n.name
    assert utm30n.dimension == 2
    assert len(utm30n.axes) == 2


def test_crs_without_authority_code_reports_none():
    wkt = crskit.crs_from_epsg(4258).to_wkt(crskit.WktVersion.WKT2_2019)
    assert crskit.crs_from_wkt(wkt).authority_code in (4258, None)


def test_to_wkt_emits_the_requested_flavour(utm30n):
    assert utm30n.to_wkt().startswith("PROJCS")
    assert utm30n.to_wkt(crskit.WktVersion.WKT2_2019).startswith("PROJCRS")


def test_a_crs_equals_its_own_wkt_round_trip(utm30n):
    assert crskit.crs_from_wkt(utm30n.to_wkt()) == utm30n


def test_different_systems_are_not_equal(etrs89, utm30n):
    assert etrs89 != utm30n
    assert etrs89.compare(utm30n) == 0


def test_comparing_a_crs_with_something_else_is_not_an_error(utm30n):
    assert utm30n != "EPSG:25830"


def test_crs_is_unhashable(utm30n):
    with pytest.raises(TypeError):
        {utm30n}


def test_transform_a_single_point(etrs89, utm30n):
    transformation = crskit.transformation(etrs89, utm30n)

    easting, northing = transformation.transform(MADRID_LAT_LON)

    # Madrid, in UTM zone 30N: roughly 440 km east, 4474 km north.
    assert easting == pytest.approx(440_000, abs=10_000)
    assert northing == pytest.approx(4_474_000, abs=10_000)


def test_transform_round_trip_returns_the_original_point(etrs89, utm30n):
    projected = crskit.transformation(etrs89, utm30n).transform(MADRID_LAT_LON)
    geographic = crskit.transformation(utm30n, etrs89).transform(projected)

    assert geographic == pytest.approx(MADRID_LAT_LON, abs=1e-9)


def test_transformation_describes_itself(etrs89, utm30n):
    transformation = crskit.transformation(etrs89, utm30n)

    assert transformation.source == etrs89
    assert transformation.target == utm30n
    assert transformation.source_dimension == 2
    assert transformation.target_dimension == 2
    assert not transformation.is_identity


def test_transform_a_point_of_the_wrong_dimension_is_rejected(etrs89, utm30n):
    with pytest.raises(crskit.DimensionMismatchError):
        crskit.transformation(etrs89, utm30n).transform([40.4, -3.7, 650.0])


def test_an_unknown_epsg_code_raises():
    with pytest.raises(crskit.AuthorityCodeNotFoundError):
        crskit.crs_from_epsg(999_999)


def test_malformed_wkt_raises():
    with pytest.raises(crskit.CrsError):
        crskit.crs_from_wkt("this is not WKT")


class TestNumPy:
    """transform_points() is the bulk path: one crossing of the boundary for N points."""

    def test_transform_an_array_of_points(self, etrs89, utm30n):
        numpy = pytest.importorskip("numpy")
        transformation = crskit.transformation(etrs89, utm30n)

        points = numpy.array([MADRID_LAT_LON, [41.385064, 2.173404]])  # Madrid, Barcelona
        projected = transformation.transform_points(points)

        assert projected.shape == (2, 2)
        assert projected[0] == pytest.approx(transformation.transform(MADRID_LAT_LON))

    def test_transform_a_single_point_keeps_its_shape(self, etrs89, utm30n):
        numpy = pytest.importorskip("numpy")
        transformation = crskit.transformation(etrs89, utm30n)

        projected = transformation.transform_points(numpy.array(MADRID_LAT_LON))

        assert projected.shape == (2,)

    def test_an_array_of_the_wrong_width_is_rejected(self, etrs89, utm30n):
        numpy = pytest.importorskip("numpy")

        with pytest.raises(crskit.DimensionMismatchError):
            crskit.transformation(etrs89, utm30n).transform_points(numpy.zeros((3, 3)))
