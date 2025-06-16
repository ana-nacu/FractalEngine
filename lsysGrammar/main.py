from grammar import generate_lsystem, is_parametric, is_stochastic, is_standard
from generator import process_lsystems_in_folder
import os
import time
import csv

def benchmark_lsystem_generation(name, axiom, rules, constants, iterations, output_csv):
    with open(output_csv, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["iteration", "generate_time_ms", "length"])

        for i in iterations:
            start = time.perf_counter()
            result = generate_lsystem(axiom, rules, i, constants)
            end = time.perf_counter()

            gen_time_ms = (end - start) * 1000
            final_string = result[-1] if isinstance(result, list) else result
            length = len(final_string)

            writer.writerow([i, round(gen_time_ms, 3), length])

    print(f"✅ Benchmark saved to {output_csv}")

def benchmark_full_generation(name, axiom, rules, constants, iterations, output_dir_txt, output_dir_obj, output_csv):
    import shutil

    os.makedirs(output_dir_txt, exist_ok=True)
    os.makedirs(output_dir_obj, exist_ok=True)

    with open(output_csv, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["iteration", "generate_ms", "txt_write_ms", "obj_gen_ms", "total_ms", "string_length"])

        for i in iterations:
            # 1. Generare regulă
            t0 = time.perf_counter()
            result = generate_lsystem(axiom, rules, i, constants)
            t1 = time.perf_counter()

            final_string = result[-1] if isinstance(result, list) else result
            lsys_txt = f"{name}__iter_{i}.txt"
            lsys_txt_path = os.path.join(output_dir_txt, lsys_txt)

            # 2. Scriere fișier .txt
            with open(lsys_txt_path, "w") as txt_f:
                txt_f.write(final_string)
            t2 = time.perf_counter()

            # 3. Generare .obj
            t3 = time.perf_counter()
            process_lsystems_in_folder(output_dir_txt, output_dir_obj)
            t4 = time.perf_counter()

            generate_ms = (t1 - t0) * 1000
            txt_write_ms = (t2 - t1) * 1000
            obj_gen_ms = (t4 - t3) * 1000
            total_ms = (t4 - t0) * 1000

            writer.writerow([i, round(generate_ms, 2), round(txt_write_ms, 2), round(obj_gen_ms, 2), round(total_ms, 2), len(final_string)])

    print(f"✅ Benchmark complet salvat în {output_csv}")

#lsystems = {

    # "SimpleTree": {
    #     "axiom": "A(10)",
    #     "rules": ["A(s) -> F(s)[+A(s/1.456)][-A(s/1.456)][^A(s/1.456)][&A(s/1.456)]"],
    #     "constants": {
    #         "R": 1.456
    #     }
    # },
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
#}
lsystems = {
    # "FernParametric": {
    #     "axiom": "A(7)",
    #     "rules": [
    #         "A(d) : d > 0 -> A(d - 1)",
    #         "A(d) : d == 0 -> F(1)[+A(D)][-A(D)]F(1)A(0)",
    #         "F(a) -> F(a * R)"
    #     ],
    #     "constants": {
    #         "D": 1,
    #         "R": 1.17
    #     },
    #     "angle": 25,
    #     "random_y_rotation": True
    # },
# "SpiralGuardian": {
#     "axiom": "A(6)",
#     "rules": [
#         "A(d) : d > 0 -> F(0.4)[+A(d-1)][^A(d-1)][&A(d-1)][-A(d-1)]",
#         "A(d) : d == 0 -> F(0.4)"
#     ],
#     "constants": {},
#     "angle": 32,
#     "random_y_rotation": True
# },
# "EntropicTitan": {
#     "axiom": "F",
#     "rules": [
#         "0.25 -> F -> F[+F]F[-F]^F",
#         "0.25 -> F -> F[+F][-F]&F",
#         "0.25 -> F -> F[-F]F[+F]^F",
#         "0.25 -> F -> F"
#     ],
#     "constants": {},
#     "angle": 27,
#     "random_y_rotation": True
# },
# "RoyalPalm": {
#     "axiom": "A(6)",
#     "rules": [
#         "A(d) : d > 1 -> F(0.3)A(d-1)",
#         "A(d) : d == 1 -> F(0.3)[+F(0.4)][-F(0.4)][&F(0.4)][^F(0.4)][F(0.4)]"
#     ],
#     "constants": {},
#     "angle": 20,
#     "random_y_rotation": True
# }.
# "SkyrootWanderer": {
#     "axiom": "A(4)",
#     "rules": [
#         "A(d) : d > 0 -> F(0.5)[+A(d-1)][-A(d-1)][&F(0.3)][^F(0.3)]",
#         "A(d) : d == 0 -> F(0.5)"
#     ],
#     "constants": {},
#     "angle": 35,
#     "random_y_rotation": True
# },
# "OakSentinel": {
#     "axiom": "A(6)",
#     "rules": [
#         "A(d) : d > 1 -> F(0.5)A(d - 1)",
#         "A(d) : d == 1 -> F(0.4)[+B(3)][-B(3)][&B(3)][^B(3)]",
#         "B(d) : d > 0 -> F(0.3)[+B(d - 1)][-B(d - 1)]",
#         "B(d) : d == 0 -> F(0.2)"
#     ],
#     "constants": {},
#     "angle": 28,
#     "random_y_rotation": True
# }
# "DragonUmbrella": {
#     "axiom": "A(6)",
#     "rules": [
#         "A(d) : d > 1 -> F(0.6)A(d - 1)",
#         "A(d) : d == 1 -> [^F(0.5)][&F(0.5)][+F(0.5)][-F(0.5)][^F(0.5)][&F(0.5)]"
#     ],
#     "constants": {},
#     "angle": 18,
#     "random_y_rotation": True
# }
# "HighlandOak": {
#     "axiom": "A(5)",
#     "rules": [
#         "A(d) : d > 1 -> F(0.6)A(d - 1)",
#         "A(d) : d == 1 -> F(0.3)[+B(2)][-B(2)][&B(2)][^B(2)][+B(2)][-B(2)]",
#         "B(d) : d > 0 -> F(0.3)[+B(d - 1)][-B(d - 1)][&B(d - 1)][^B(d - 1)]",
#         "B(d) : d == 0 -> F(0.2)"
#     ],
#     "constants": {},
#     "angle": 25,
#     "random_y_rotation": True
# },
# "AlpinePine": {
#     "axiom": "F",
#     "rules": [
#         "F -> F[+F][-F][&F][^F]FF"
#     ],
#     "constants": {},
#     "angle": 20,
#     "random_y_rotation": True
# },
# "PinOakMajestic": {
#     "axiom": "A(6)",
#     "rules": [
#         "A(d) : d > 1 -> F(0.7)A(d - 1)",
#         "A(d) : d == 1 -> F(0.3)[+B(3)][-B(3)][&B(3)][^B(3)][+B(3)][-B(3)][&B(3)]",
#         "B(d) : d > 0 -> F(0.25)[+B(d - 1)][-B(d - 1)][^B(d - 1)][&B(d - 1)]",
#         "B(d) : d == 0 -> F(0.2)"
#     ],
#     "constants": {},
#     "angle": 22,
#     "random_y_rotation": True
# },
#     "GoldenPineTree": {
#         "axiom": "A(1.0)",
#         "rules": [
#             "A(a) : a > 0.05 -> F(a)"
#             "[+F(a * rB)A(a * rB)]"
#             "[-F(a * rB)A(a * rB)]"
#             "[^F(a * rB)A(a * rB)]"
#             "[&F(a * rB)A(a * rB)]"
#             "F(a * rA)""A(a * rA)",
#             "F(x) -> F(x)"
#         ],
#         "constants": {
#             "rA": 0.618,  # vertical continuation
#             "rB": 0.382   # side branches
#         },
#         "angle": 72,
#         "random_y_rotation": True
#     }
# "FractalBinaryTree": {
#     "axiom": "F(1.0)A(1.0)",
#     "rules": [
#         "A(a) : a > 0.05 -> "
#         "[+^F(a * r1)A(a * r1)]"
#         "[-&F(a * r)A(a * r)]",
#         "F(x) -> F(x)"
#     ],
#     "constants": {
#         "r": 0.707,
#         "r1": 0.35
#     },
#     "angle": 75,
#     "random_y_rotation": True
# },
# "RandomDream": {
#     "axiom": "F",
#     "rules": [
#         "0.5 -> F -> [+F]F[-F]^F",
#         "0.25 -> F -> F[+F][-F]&F",
#         "0.25 -> F -> F"
#     ],
#     "constants": {},
#     "angle": 67,
#     "random_y_rotation": True
 # },
#     "ThunderstruckGiant": {
#         "axiom": "+F(2.0)A(2.0)",
#         "rules": [
#             "A(a) : a > 0.05 -> "
#             "[+^F(a * r)A(a * r1)]"
#             "[+&F(a * r1)A(a * r)][+&&F(a * r1)A(a * r)]",
#             "F(x) -> F(x)"
#         ],
#         "constants": {
#             "r": 0.707,
#             "r1": 0.35
#         },
#         "angle": 20,
#         "random_y_rotation": True
#     },
    "GoldenOaktree": {
        "axiom": "A(1.0)",
        "rules": [
            "A(a) : a > 0.05 -> F(a)"
            "[+A(a * rB)]"
            "[-A(a * rB)]"
            "[^A(a * rB)]"
            "[&A(a * rB)]"
            "A(a * rA)",
            "F(x) -> F(x)"
        ],
        "constants": {
            "rA": 0.618,  # vertical continuation
            "rB": 0.382  # side branches
        },
        "angle": 72,
        "random_y_rotation": True
    }
}
output_dir_txt = "GoldenOaktree"
output_dir_obj = "GoldenOaktree_obj"
# output_dir_txt = "benchmark_lsystems"
# output_dir_obj = "benchmark_lsystems_obj"
os.makedirs(output_dir_txt, exist_ok=True)
os.makedirs(output_dir_obj, exist_ok=True)

# 1. Scriem fișiere .txt
for name, data in lsystems.items():
    axiom = data["axiom"]
    rules = data["rules"]
    constants = data.get("constants", {})
    result = generate_lsystem(axiom, rules, 7, constants)

    rule_type = (
        "parametric" if is_parametric(rules)
        else "stochastic" if is_stochastic(rules)
        else "standard"
    )

    for i, iteration in enumerate(result):
        filename = f"{name}__{rule_type}__iter_{i}.txt"
        with open(os.path.join(output_dir_txt, filename), "w") as f:
            f.write(f"#params: angle={data.get('angle', 25)}\n")
            if data.get("random_y_rotation"):
                f.write(f"#params: rotation_y=random\n")
            f.write(iteration)


print(f"✅ Saved {len(lsystems)} L-systems with 10 iterations each in '{output_dir_txt}/'")

# 2. Procesăm toate fișierele din folder
process_lsystems_in_folder(output_dir_txt, output_dir_obj)

# benchmark_full_generation(
#     name="SimpleTree",
#     axiom=lsystems["SimpleTree"]["axiom"],
#     rules=lsystems["SimpleTree"]["rules"],
#     constants=lsystems["SimpleTree"]["constants"],
#     iterations=range(1, 9),
#     output_dir_txt="benchmark_lsystems",
#     output_dir_obj = "benchmark_lsystems_obj",
#     output_csv="benchmark_full_generation.csv"
# )