# tests_py/test_generator_math.py

import numpy as np
import math
import pytest
from lsysGrammar.generator import normalize, rotation_matrix

def test_normalize_zero_vector():
    z = np.array([0.0,0.0,0.0])
    assert np.allclose(normalize(z), z)

def test_normalize_nonzero_vector():
    v = np.array([3.0,4.0,0.0])
    n = normalize(v)
    assert pytest.approx(np.linalg.norm(n), rel=1e-6) == 1.0

def test_rotation_matrix_orthonormal_and_rotate():
    axis = [0,0,1]
    angle = math.pi/2
    R = rotation_matrix(axis, angle)
    # R should be orthonormal: R·R^T = I
    assert np.allclose(R.dot(R.T), np.eye(3), atol=1e-6)
    # rotating (1,0,0) by 90° around z should give (0,1,0)
    v = np.array([1.0,0.0,0.0])
    out = R.dot(v)
    assert np.allclose(out, [0.0,1.0,0.0], atol=1e-6)