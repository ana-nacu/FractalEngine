# tests_py/test_generate_branch.py

import numpy as np
from lsysGrammar.generator import generate_branch

def test_generate_branch_counts():
    start = np.array([0,0,0])
    direction = np.array([0,1,0])
    length = 1.0
    verts, faces = generate_branch(start, direction, length, num_sides=8)
    # should be 2 rings of 8 verts each
    assert len(verts) == 16
    # each of the 8 sides generates 2 triangles
    assert len(faces) == 8 * 2
    # all face indices should be in [0,15]
    all_idxs = {i for tri in faces for i in tri}
    assert all(i in all_idxs for i in range(16))