"""Tests for the Python binding.

The EPSG database comes from DIGI21_EPSG_SQLITE, exactly as in the C++ (GoogleTest) suite, or from
the crskit-epsg package when it is installed. Without either, the suite skips itself.
"""

import os
import struct
from pathlib import Path

import pytest

import crskit


def _epsg_database() -> str | None:
    if database := os.environ.get("DIGI21_EPSG_SQLITE"):
        return database

    try:
        import crskit_epsg
    except ImportError:
        return None

    return str(crskit_epsg.database_path())


EPSG_DATABASE = _epsg_database()

# EPSG 4258 (ETRS89, geographic 2D, latitude/longitude) and 25830 (ETRS89 / UTM zone 30N).
MADRID_LAT_LON = [40.416775, -3.703790]


@pytest.fixture(scope="session", autouse=True)
def initialise():
    if EPSG_DATABASE is None or not os.path.exists(EPSG_DATABASE):
        pytest.skip("no EPSG database: set DIGI21_EPSG_SQLITE or install crskit-epsg")
    crskit.init(EPSG_DATABASE)


@pytest.fixture(scope="session")
def etrs89():
    return crskit.crs_from_epsg(4258)


@pytest.fixture(scope="session")
def utm30n():
    return crskit.crs_from_epsg(25830)


def test_the_library_reports_itself_initialized():
    assert crskit.is_initialized()


def test_the_package_ships_its_type_information():
    # PEP 561: without these two inside the installed package, editors and mypy see the compiled
    # module as an untyped black box.
    package = Path(crskit.__file__).parent
    assert (package / "py.typed").exists()
    assert (package / "_crskit.pyi").exists()


def test_epsg_version_is_reported():
    assert crskit.epsg_version()


def test_init_without_arguments_finds_the_data_package(monkeypatch):
    crskit_epsg = pytest.importorskip("crskit_epsg")
    monkeypatch.delenv("DIGI21_EPSG_SQLITE", raising=False)

    crskit.init()

    assert crskit.epsg_version() == crskit_epsg.EPSG_VERSION

    crskit.init(EPSG_DATABASE)  # leave the suite on the database it was given


def test_init_without_a_database_anywhere_says_what_to_do(monkeypatch):
    monkeypatch.delenv("DIGI21_EPSG_SQLITE", raising=False)
    monkeypatch.setitem(__import__("sys").modules, "crskit_epsg", None)

    with pytest.raises(crskit.CrsError, match="crskit\\[epsg\\]"):
        crskit.init()

    crskit.init(EPSG_DATABASE)


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


def test_an_ambiguous_pair_without_select_operation_raises_multiple_transformations_found():
    # The European Petroleum Survey Group (EPSG) catalogue defines several coordinate operations from European Datum
    # 1950 (EPSG:4230) to European Terrestrial Reference System 1989 (EPSG:4258).
    with pytest.raises(crskit.MultipleTransformationsFoundError) as exception_info:
        crskit.transformation(crskit.crs_from_epsg(4230), crskit.crs_from_epsg(4258))
    assert isinstance(exception_info.value, crskit.TransformationNotFoundError)


def _candidate_operations(source_code, target_code, chosen_code):
    """The candidate operations that transformation() passes to select_operation, which returns chosen_code."""
    candidates = []

    def select_operation_by_code(source_name, target_name, operations):
        candidates.extend(operations)
        return chosen_code

    try:
        crskit.transformation(crskit.crs_from_epsg(source_code), crskit.crs_from_epsg(target_code), select_operation=select_operation_by_code)
    except crskit.GridFileNotFoundError:
        pass
    return {operation.code: operation for operation in candidates}


def test_area_of_use_bounding_boxes_of_operations_1311_and_1633():
    # Among the 37 datum shifts from European Datum 1950 (ED50, EPSG:4230) to World Geodetic System 1984 (WGS 84,
    # EPSG:4326), the area of use of EPSG 1311 (Europe - common offshore) does not contain Madrid and the area of use of
    # EPSG 1633 (Spain - mainland except northwest) does.
    operations = _candidate_operations(4230, 4326, 1633)

    operation_1311_bounding_box = operations[1311].area_of_use_bounding_box
    assert operation_1311_bounding_box.south_bound_latitude_degrees == 47.42
    assert operation_1311_bounding_box.north_bound_latitude_degrees == 63.89
    assert operation_1311_bounding_box.west_bound_longitude_degrees == -16.1
    assert operation_1311_bounding_box.east_bound_longitude_degrees == 10.86
    assert not operation_1311_bounding_box.contains(40.41, -3.71)

    operation_1633_bounding_box = operations[1633].area_of_use_bounding_box
    assert operation_1633_bounding_box.contains(40.41, -3.71)
    assert not operation_1633_bounding_box.crosses_antimeridian


def test_bounding_box_crossing_the_antimeridian_contains_points_on_both_sides_inside_its_bounds():
    # EPSG 1254 (Pulkovo 1942 to WGS 84 (1)), one of the 13 operations from EPSG:4284 to EPSG:4326, Russia - onshore:
    # from longitude 19.58 degrees eastward to -168.97 degrees, across the antimeridian.
    operation_1254_bounding_box = _candidate_operations(4284, 4326, 1254)[1254].area_of_use_bounding_box

    assert operation_1254_bounding_box.west_bound_longitude_degrees == 19.58
    assert operation_1254_bounding_box.east_bound_longitude_degrees == -168.97
    assert operation_1254_bounding_box.crosses_antimeridian
    assert operation_1254_bounding_box.contains(66.0, 170.0)
    assert operation_1254_bounding_box.contains(66.0, 180.0)
    assert operation_1254_bounding_box.contains(66.0, -172.0)
    assert not operation_1254_bounding_box.contains(66.0, -160.0)
    assert not operation_1254_bounding_box.contains(66.0, 0.0)


def _write_synthetic_egm2008_grid(grid_file_path, undulation):
    # 101 rows of 200 little-endian floats, each row between two 4-byte FORTRAN record markers: 81608 bytes.
    grid_file_path.write_bytes(struct.pack("<202f", 0.0, *([undulation] * 200), 0.0) * 101)


def test_grid_files_are_found_with_the_search_of_the_transformation(tmp_path):
    # EPSG 3859 (WGS 84 to EGM2008 height (2)) needs the 1 arc-minute EGM2008 grid, which the test writes to an empty
    # data directory.
    grid_file_name = "Und_min1x1_egm2008_isw=82_WGS84_TideFree"
    crskit.init(EPSG_DATABASE, tmp_path)
    try:
        geoid_height_operation = _candidate_operations(4979, 3855, 3859)[3859]
        assert geoid_height_operation.grid_files == [grid_file_name]
        assert not crskit.is_grid_file_installed(grid_file_name)
        assert crskit.missing_grid_files(geoid_height_operation) == [grid_file_name]

        _write_synthetic_egm2008_grid(tmp_path / grid_file_name, 30.0)
        assert crskit.is_grid_file_installed(grid_file_name)
        assert crskit.is_grid_file_installed(grid_file_name + ".gz")
        assert crskit.missing_grid_files(geoid_height_operation) == []

        transformation = crskit.transformation(
            crskit.crs_from_epsg(4979), crskit.crs_from_epsg(3855), select_operation=lambda source_name, target_name, operations: 3859
        )
        assert transformation.transform([0.0, 0.0, 100.0])[2] == pytest.approx(70.0, abs=1e-3)
    finally:
        crskit.init(EPSG_DATABASE)


def test_missing_grid_files_is_empty_for_norway_offshore_interpolation():
    # EPSG 1450 uses the method Norway Offshore Interpolation (EPSG 9620): its grid_files are names of operations.
    norway_offshore_operation = _candidate_operations(4230, 4326, 1633)[1450]
    assert norway_offshore_operation.grid_files
    assert crskit.missing_grid_files(norway_offshore_operation) == []


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
