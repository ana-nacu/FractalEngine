import os
import math
import re
import numpy as np
import open3d as o3d


def normalize(v):
    norm = np.linalg.norm(v)
    return v / norm if norm > 0 else v


def rotation_matrix(axis, angle):
    axis = normalize(axis)
    cos_a = math.cos(angle)
    sin_a = math.sin(angle)
    ux, uy, uz = axis
    return np.array([
        [cos_a + ux**2 * (1 - cos_a), ux * uy * (1 - cos_a) - uz * sin_a, ux * uz * (1 - cos_a) + uy * sin_a],
        [uy * ux * (1 - cos_a) + uz * sin_a, cos_a + uy**2 * (1 - cos_a), uy * uz * (1 - cos_a) - ux * sin_a],
        [uz * ux * (1 - cos_a) - uy * sin_a, uz * uy * (1 - cos_a) + ux * sin_a, cos_a + uz**2 * (1 - cos_a)]
    ])


def generate_branch(start, direction, length, num_sides=6):
    vertices = []
    faces = []

    end = start + direction * length
    radius = length * 0.05

    if np.allclose(direction, [0, 1, 0]):
        ortho = np.array([1.0, 0.0, 0.0])
    else:
        ortho = normalize(np.cross(direction, [0.0, 1.0, 0.0]))

    bitangent = normalize(np.cross(direction, ortho))

    ring_start = []
    ring_end = []

    for i in range(num_sides):
        theta = 2 * math.pi * i / num_sides
        offset = radius * (math.cos(theta) * ortho + math.sin(theta) * bitangent)
        ring_start.append(start + offset)
        ring_end.append(end + offset)

    base_index = len(vertices)
    vertices.extend(ring_start)
    vertices.extend(ring_end)

    for i in range(num_sides):
        i1 = base_index + i
        i2 = base_index + (i + 1) % num_sides
        i3 = base_index + num_sides + i
        i4 = base_index + num_sides + (i + 1) % num_sides

        faces.append([i3, i2, i1])
        faces.append([i4, i2, i3])

    return vertices, faces


def interpret_lsystem_3d(rule_str, rule_type="standard", angle_deg=30):
    position = np.array([0.0, 0.0, 0.0])
    direction = np.array([0.0, 1.0, 0.0])
    stack = []
    branches = []
    angle = math.radians(angle_deg)

    param_pattern = re.compile(r'([A-Z])\((-?[0-9]*\.?[0-9]+)\)')
    i = 0
    while i < len(rule_str):
        c = rule_str[i]
        if rule_type == "parametric" and rule_str[i:].startswith('F('):
            match = param_pattern.match(rule_str[i:])
            if match:
                length = float(match.group(2))
                new_pos = position + direction * length
                branches.append((position.copy(), new_pos.copy()))
                position = new_pos
                i += len(match.group(0))
                continue

        if c in ['F', 'X'] and rule_type != "parametric":
            new_pos = position + direction * 0.05
            branches.append((position.copy(), new_pos.copy()))
            position = new_pos
        elif c == '+':
            direction = rotation_matrix([0, 0, 1], angle) @ direction
        elif c == '-':
            direction = rotation_matrix([0, 0, 1], -angle) @ direction
        elif c == '^':
            direction = rotation_matrix([1, 0, 0], angle) @ direction
        elif c == '&':
            direction = rotation_matrix([1, 0, 0], -angle) @ direction
        elif c == '[':
            stack.append((position.copy(), direction.copy()))
        elif c == ']':
            position, direction = stack.pop()
        i += 1

    return branches


def export_obj(branches, output_path, num_sides=6, decimate=True):
    all_vertices = []
    all_faces = []
    index_offset = 0

    for start, end in branches:
        verts, faces = generate_branch(
            np.array(start),
            normalize(np.array(end) - np.array(start)),
            np.linalg.norm(np.array(end) - np.array(start)),
            num_sides
        )
        all_vertices.extend(verts)
        all_faces.extend([[idx + index_offset for idx in face] for face in faces])
        index_offset += len(verts)

    if not all_vertices:
        print(f"⚠️ Mesh gol pentru {output_path}, se sare peste.")
        return

    mesh = o3d.geometry.TriangleMesh()
    mesh.vertices = o3d.utility.Vector3dVector(all_vertices)
    mesh.triangles = o3d.utility.Vector3iVector(all_faces)

    # 🧼 Clean up înainte de decimare
    mesh.remove_duplicated_vertices()
    mesh.remove_degenerate_triangles()
    mesh.remove_duplicated_triangles()
    mesh.remove_non_manifold_edges()

    if decimate:
        target_triangles = max(10, int(len(all_faces) * 0.95))
        mesh = mesh.simplify_quadric_decimation(target_number_of_triangles=target_triangles)

        # 🧠 Recomandat: recalcul normalele după decimare
        mesh.compute_vertex_normals()
        mesh.compute_triangle_normals()
    else:
        mesh.compute_vertex_normals()
        mesh.compute_triangle_normals()

    # 💾 Salvare robustă, inclusiv normalele
    o3d.io.write_triangle_mesh(
        output_path,
        mesh,
        write_vertex_normals=True,
        write_triangle_uvs=False,
        compressed=False
    )

    print(f"✅ Salvat (cu Open3D): {output_path}")

def process_lsystems_in_folder(input_folder, output_folder):
    os.makedirs(output_folder, exist_ok=True)
    for filename in os.listdir(input_folder):
        if filename.endswith(".txt"):
            print(filename)
            with open(os.path.join(input_folder, filename), "r") as f:
                rule = f.read().strip()
            rule_type = "parametric" if "parametric" in filename else (
                "stochastic" if "stochastic" in filename else "standard")
            branches = interpret_lsystem_3d(rule, rule_type)
            output_file = os.path.splitext(filename)[0] + ".obj"
            export_obj(branches, os.path.join(output_folder, output_file))