# tests_py/test_interpret_lsystem_3d.py

import numpy as np
from lsysGrammar.generator import interpret_lsystem_3d

def test_interpret_simple_forward():
    # “F” with default rule_type should move 0.05 in +y
    branches = interpret_lsystem_3d("F", rule_type="standard")
    assert len(branches) == 1
    p0, p1 = branches[0]
    assert np.allclose(p1 - p0, [0,0.05,0], atol=1e-6)

def test_interpret_brackets_stack():
    # “F[F]F” should produce three segments, and return to original dir after ]
    branches = interpret_lsystem_3d("F[F]F", rule_type="standard")
    assert len(branches) == 3