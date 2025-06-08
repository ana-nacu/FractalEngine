# tests_py/test_generator.py

import pytest
import re
from lsysGrammar.grammar import (count_substring, is_stochastic, is_standard, is_parametric,
                                 apply_rule,generate_lsystem)

# a minimal “standard” rule set
STANDARD_RULES = ["X -> AX"]
# a minimal “parametric” rule set
PARAM_RULES= ["A(d) : d>0 -> A(d-1)"]
# a minimal “stochastic” rule set
STOCH_RULES= ["0.5 -> F -> F+", "0.5 -> F -> F-"]

def test_apply_standard():
    assert apply_rule("X", STANDARD_RULES, {}) == "AX"
    assert apply_rule("Y", STANDARD_RULES, {}) == "Y"

def test_apply_parametric():
    out = apply_rule("A(3)", PARAM_RULES, {})
    # A(3)->A(2)
    assert re.match(r"A\(2(\.00)?\)", out)

def test_apply_stochastic():
    # we just check it never crashes and returns either "+" or "-"
    result = set(apply_rule("F", STOCH_RULES, {}) for _ in range(20))
    assert all(r.endswith("+") or r.endswith("-") for r in result)
def test_generate_lsystem_growth():
    seq = generate_lsystem("X", STANDARD_RULES, iterations=3, params={})
    # X→AB→ABB→ABBB  (length grows by 1 each iter)
    assert seq == ["X", "AX", "AAX", "AAAX"]

def test_generate_parametric_iterations():
    seq = generate_lsystem("A(2)", PARAM_RULES, iterations=2, params={})
    # A(2)->A(1)->A(0)
    assert seq[0] == "A(2)"
    assert re.match(r"A\(1(\.00)?\)", seq[1])
    assert re.match(r"A\(0(\.00)?\)", seq[2])