# tests_py/test_export_and_process.py

import os
import tempfile
import numpy as np
from lsysGrammar.generator import export_obj, interpret_lsystem_3d, process_lsystems_in_folder

def test_export_obj_creates_file(tmp_path):
    branches = [ (np.array([0,0,0]), np.array([0,1,0])) ]
    out = tmp_path / "single_branch.obj"
    export_obj(branches, str(out), num_sides=4, decimate=False)
    assert out.exists()
    text = out.read_text()
    # basic sanity: should contain “v ” lines and “f ” lines
    assert text.count("\nv ") >= 4
    assert "f " in text

def test_process_lsystems_in_folder(tmp_path):
    # write a simple .txt with rule “F”
    src = tmp_path / "in"
    dst = tmp_path / "out"
    src.mkdir()
    (src / "X__standard__iter_0.txt").write_text("F")
    process_lsystems_in_folder(str(src), str(dst))
    # expect one .obj in dst
    files = list(dst.glob("*.obj"))
    assert len(files) == 1