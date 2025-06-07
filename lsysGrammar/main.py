from grammar import generate_lsystem, is_parametric, is_stochastic, is_standard
from generator import process_lsystems_in_folder
import os

lsystems = {
    "SimpleTree": {
        "axiom": "A(10)",
        "rules": ["A(s) -> F(s)[+A(s/1.456)][-A(s/1.456)][^A(s/1.456)][&A(s/1.456)]"],
        "constants": {
            "R": 1.456
        }
    },
    # "Tree": {
    #     "axiom": "X",
    #     "rules": ["X -> F-[[X]+X]+F[+FX]-X", "F -> FF"],
    #     "constants": {}
    # },
    # "StochasticTree": {
    #     "axiom": "F",
    #     "rules": [
    #         "0.5 -> F -> F[+F]F[-F]F",
    #         "0.5 -> F -> F[+F]F"
    #     ],
    #     "constants": {}
    # },
    # "FernParametric": {
    #     "axiom": "A(7)",  # Poți încerca și A(3) pentru o ferigă mai mare
    #     "rules": [
    #         "A(d) : d > 0 -> A(d - 1)",
    #         "A(d) : d == 0 -> F(1)[+A(D)][-A(D)][&A(D)][^A(D)]F(1)A(0)",
    #         "F(a) -> F(a * R)"
    #     ],
    #     "constants": {
    #         "D": 1,
    #         "R": 1.17
    #     }
    # }
}

output_dir_txt = "treeB"
output_dir_obj = "treeB_generated_objs"
os.makedirs(output_dir_txt, exist_ok=True)
os.makedirs(output_dir_obj, exist_ok=True)

# # 1. Scriem fișiere .txt
# for name, data in lsystems.items():
#     axiom = data["axiom"]
#     rules = data["rules"]
#     constants = data.get("constants", {})
#     result = generate_lsystem(axiom, rules, 9, constants)
#
#     rule_type = (
#         "parametric" if is_parametric(rules)
#         else "stochastic" if is_stochastic(rules)
#         else "standard"
#     )
#
#     for i, iteration in enumerate(result):
#         filename = f"{name}__{rule_type}__iter_{i}.txt"
#         with open(os.path.join(output_dir_txt, filename), "w") as f:
#             f.write(iteration)
#
#
# print(f"✅ Saved {len(lsystems)} L-systems with 10 iterations each in '{output_dir_txt}/'")

# 2. Procesăm toate fișierele din folder
process_lsystems_in_folder(output_dir_txt, output_dir_obj)