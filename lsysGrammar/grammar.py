import re
import json
import random
from typing import List, Dict

# ------------------------ Rule Type Detectors ------------------------

def count_substring(s: str, sub: str) -> int:
    return s.count(sub)

def is_stochastic(rules: List[str]) -> bool:
    if len(rules) == 1:
        return False
    parts = rules[0].split("->")
    return len(parts) == 3 and count_substring(rules[0], "->") == 2

def is_standard(rules: List[str]) -> bool:
    parts = rules[0].split("->")
    return len(parts) == 2 and ":" not in rules[0] and "(" not in rules[0]

def is_parametric(rules: List[str]) -> bool:
    parts = rules[0].split("->")
    return len(parts) == 2 and "(" in rules[0]

# ------------------------ L-System Generator ------------------------

def apply_rule(symbol: str, rules: List[str], params: Dict[str, float]) -> str:
    for rule in rules:
        if is_parametric([rule]):
            match = re.match(r"([A-Z])\(([^)]*)\)\s*(?::\s*(.+?))?\s*->\s*(.+)", rule)
            if match and symbol.startswith(match.group(1) + "("):
                pred_letter = match.group(1)
                param_names = match.group(2).split(",")
                condition = match.group(3)  # can be None
                successor = match.group(4)

                args = symbol[symbol.find("(")+1:symbol.find(")")].split(",")
                if len(args) != len(param_names):
                    continue

                local_env = dict(params)
                for name, val in zip(param_names, args):
                    try:
                        local_env[name.strip()] = float(val)
                    except:
                        pass

                if condition:
                    try:
                        if not eval(condition, {}, local_env):
                            continue
                    except:
                        continue

                def eval_expr(expr: str) -> str:
                    try:
                        return "{:.2f}".format(eval(expr.strip(), {}, local_env))
                    except:
                        return expr

                # EXTINS: permite și simboluri direcționale cu parametri: +(), -(), ^(), etc.
                result = re.sub(
                    r"([A-Za-z\+\-\&\^\|\\/])\(([^)]+)\)",
                    lambda m: f"{m.group(1)}({eval_expr(m.group(2))})",
                    successor
                )
                return result

        elif is_standard([rule]):
            lhs, rhs = map(str.strip, rule.split("->"))
            if symbol == lhs:
                return rhs

        elif is_stochastic(rules):
            stochastic_rules = []
            for rule in rules:
                parts = rule.split("->")
                if len(parts) == 3:
                    try:
                        probability = float(parts[0].strip())
                        symbol_match = parts[1].strip()
                        replacement = parts[2].strip()
                        if symbol == symbol_match:
                            stochastic_rules.append((probability, symbol_match, replacement))
                    except ValueError:
                        continue

            total_prob = sum(p[0] for p in stochastic_rules)
            r = random.uniform(0, total_prob)
            cumulative = 0.0
            for prob, sym, repl in stochastic_rules:
                cumulative += prob
                if r <= cumulative:
                    return repl

    return symbol

def generate_lsystem(axiom: str, rules: List[str], iterations: int, params: Dict[str, float]) -> List[str]:
    result = [axiom]
    current = axiom
    for _ in range(iterations):
        next_str = ""
        i = 0
        while i < len(current):
            if current[i].isalpha() or current[i] in "+-&^\\/|":
                j = i + 1
                if j < len(current) and current[j] == "(":
                    k = current.find(")", j)
                    if k != -1:
                        token = current[i:k+1]
                        next_str += apply_rule(token, rules, params)
                        i = k + 1
                        continue
                next_str += apply_rule(current[i], rules, params)
            else:
                next_str += current[i]
            i += 1
        result.append(next_str)
        current = next_str
    return result

# # ------------------------ Main Execution ------------------------
#
# lsystems = {
#     # "SimpleTree": {
#     #     "axiom": "A(50)",
#     #     "rules": ["A(s) -> F(s)[+A(s/1.456)][-A(s/1.456)][^A(s/1.456)][&A(s/1.456)]"],
#     #     "constants": {
#     #         "R": 1.456
#     #     }
#     # },
#     # "Tree": {
#     #     "axiom": "X",
#     #     "rules": ["X -> F-[[X]+X]+F[+FX]-X", "F -> FF"],
#     #     "constants": {}
#     # },
#     # "StochasticTree": {
#     #     "axiom": "F",
#     #     "rules": [
#     #         "0.5 -> F -> F[+F]F[-F]F",
#     #         "0.5 -> F -> F[+F]F"
#     #     ],
#     #     "constants": {}
#     # },
#     "FernParametric": {
#         "axiom": "A(7)",  # Poți încerca și A(3) pentru o ferigă mai mare
#         "rules": [
#             "A(d) : d > 0 -> A(d - 1)",
#             "A(d) : d == 0 -> F(1)[+A(D)][-A(D)]F(1)A(0)",
#             "F(a) -> F(a * R)"
#         ],
#         "constants": {
#             "D": 1,
#             "R": 1.17
#         }
#     }
# }
#
# output = {}
#
# for name, data in lsystems.items():
#     axiom = data["axiom"]
#     rules = data["rules"]
#     constants = data.get("constants", {})
#     output[name] = generate_lsystem(axiom, rules, 20, constants)
#
# import os
#
# output_dir = "precomputed_lsystems"
# os.makedirs(output_dir, exist_ok=True)
#
# for name, data in lsystems.items():
#     axiom = data["axiom"]
#     rules = data["rules"]
#     constants = data.get("constants", {})
#     result = generate_lsystem(axiom, rules, 20, constants)
#
#     for i, iteration in enumerate(result):
#         fractal_type = (
#             "parametric" if is_parametric(rules)
#             else "stochastic" if is_stochastic(rules)
#             else "standard"
#         )
#         filename = f"{name}__{fractal_type}__iter_{i}.txt"
#         filepath = os.path.join(output_dir, filename)
#         with open(filepath, "w") as f:
#             f.write(iteration)

# print(f"✅ Saved {len(lsystems)} L-systems with 10 iterations each in '{output_dir}/'")