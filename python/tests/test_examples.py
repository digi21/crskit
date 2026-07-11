"""The examples are documentation, so they are tested: every one of them must run.

They are written to work without the grid files (geoid models, NTv2 grids), which are licence-
restricted and cannot be shipped: instead of failing, they report which file they would need. So
they must run to completion here too, where there are no grids.
"""

import os
import subprocess
import sys
from pathlib import Path

import pytest

EXAMPLES = sorted((Path(__file__).parent.parent / "examples").glob("[0-9]*.py"))


@pytest.mark.parametrize("example", EXAMPLES, ids=lambda path: path.stem)
def test_example_runs(example: Path):
    pytest.importorskip("numpy")

    # EPSG area names carry accents and degree signs ("Norway - offshore north of 65°N"), which a
    # Windows console encodes in the ANSI codepage unless told otherwise.
    environment = os.environ | {"PYTHONIOENCODING": "utf-8"}

    result = subprocess.run(
        [sys.executable, str(example)],
        capture_output=True,
        text=True,
        encoding="utf-8",
        env=environment,
        cwd=example.parent,  # the examples import _setup from their own folder
        timeout=300,
    )

    assert result.returncode == 0, f"{example.name} failed:\n{result.stdout}\n{result.stderr}"
    assert result.stdout.strip(), f"{example.name} printed nothing"
