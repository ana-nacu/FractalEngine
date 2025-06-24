import json
import random
from math import log2, ceil

# Convertire coordonate (x, y) în index Hilbert (algoritm simplificat)
def xy2d(n, x, y):
    rx = ry = s = d = 0
    for s in reversed(range(ceil(log2(n)))):
        rx = (x >> s) & 1
        ry = (y >> s) & 1
        d += (1 << (2 * s)) * ((3 * rx) ^ ry)
        if ry == 0:
            if rx == 1:
                x = n - 1 - x
                y = n - 1 - y
            x, y = y, x
    return d

# Configurații
grid_size = 7               # dimensiunea grilei (7x7 sloturi)
slot_spacing = 7.1          # distanța dintre arbori (poate fi ajustată)
species_count = 7           # numărul total de specii disponibile
iterations = [5, 6, 7]   # iterațiile generate pentru fiecare specie

# Dimensiune minimă pătratică pentru curba Hilbert (putere a lui 2)
hilbert_n = 1 << ceil(log2(grid_size))  # ex: 8 dacă grid_size = 7

# Generare poziții (x, y) pe grid
positions = [(x, y) for y in range(grid_size) for x in range(grid_size)]

# Sortare după indexul Hilbert
positions_sorted = sorted(positions, key=lambda pos: xy2d(hilbert_n, pos[0], pos[1]))

# Generare layout scenă
scene = []
species = ["GoldenOaktree__parametric_", "GoldenPineTree__parametric_",
        "EntropicTitan__stochastic_", "RandomDream__stochastic_", "HighlandOak__parametric_",
        "SpiralGuardian__parametric_", "ThunderstruckGiant__parametric_"]
for pos in positions_sorted:
    iteration = random.choice(iterations)
    # scale = round(random.uniform(0.85, 1.15), 2)
    rotation_y = round(random.uniform(0, 360), 1)

    specie = random.choice(species)
    if specie.startswith("HighlandOak"):
        mesh_path = f"Demo/{specie}_iter_{iteration+13}.obj"
    else:
        mesh_path = f"Demo/{specie}_iter_{iteration}.obj"
    if specie.startswith("HighlandOak"):
        y=0.5
    else:
        y=0


    x = round(pos[0] * slot_spacing, 2)
    z = round(pos[1] * slot_spacing, 2)
    scene.append({
        "mesh": mesh_path,
        "position": [x, y, z],
        "scale": 1,
        "rotation_y": rotation_y
    })

# Salvare în fișier JSON
with open("FillSpace/scene_layout_hilbert_5.json", "w") as f:
    json.dump(scene, f, indent=2)