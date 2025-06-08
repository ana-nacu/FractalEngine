# tests_py/test_utils.py

import pytest
from lsysGrammar.grammar import count_substring, is_stochastic, is_standard, is_parametric
@pytest.mark.parametrize("s, sub, expected", [
    ("AAAA", "A", 4),
    ("ABABAB", "AB", 3),
    ("hello", "l", 2),
    ("", "x", 0),
])
def test_count_substring(s, sub, expected):
    assert count_substring(s, sub) == expected

def test_is_standard():
    rules = ["X -> F-[[X]+X]+F[+FX]-X"]
    assert is_standard(rules)
    assert not is_parametric(rules)
    assert not is_stochastic(rules)

def test_is_parametric():
    rules = ["A(s) -> F(s)[+A(s/1.456)]"]
    assert is_parametric(rules)
    assert not is_standard(rules)
    assert not is_stochastic(rules)

def test_is_stochastic():
    rules = ["0.5 -> F -> F[+F]F", "0.5 -> F -> F[+F]F[-F]F"]
    assert is_stochastic(rules)
    assert not is_standard(rules)
    assert not is_parametric(rules)