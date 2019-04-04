import sys
import json
from typing import List, Dict, Tuple
from math import *
import os


def parse_ns(s: str) -> float:
    s = s[len('Ns '):]
    Ns = float(s)
    return sqrt(2 / (2 + Ns))


def parse_basic_float(s: str, head: str) -> float:
    s = s[len(head):]
    return float(s)


def parse_basic_float3(s: str, head: str) -> Tuple[float, float, float]:
    s = s[len(head):]
    s = [x for x in s.split(' ') if x]
    return float(s[0]), float(s[1]), float(s[2])


def parse_filename(s: str):
    s = [x for x in s.split(' ') if x][1]
    s = s.replace('\\', '/').strip()
    return s


def load_mtl(filename) -> Dict[str, dict]:
    print('load mtl: ' + filename)
    with open(filename, 'r') as f:
        lines = f.readlines()
        materials = dict()
        i = 0
        while i < len(lines):
            line = lines[i]
            if line.startswith('newmtl'):
                mat_name = line[len('newmtl '):].strip()
                if mat_name in materials:
                    print('material ' + mat_name + 'already exists, overriding')
                materials[mat_name] = {
                    "Tr": 0,
                    "roughness": 0,
                    "ks": {
                        "texture": "",
                        "albedo": [
                            0, 0, 0
                        ]
                    },
                    "Ni": 1,
                    "ka": {
                        "texture": "",
                        "albedo": [
                            0,
                            0,
                            0
                        ]
                    },
                    "kd": {
                        "texture": "",
                        "albedo": [
                            0,
                            0,
                            0
                        ]
                    }
                }
                i += 1
                while i < len(lines):
                    line = lines[i]
                    if line.startswith('Ns'):
                        materials[mat_name]['roughness'] = parse_ns(line)
                    elif line.startswith('Pr'):
                        materials[mat_name]['roughness'] = parse_basic_float(line, 'Pr')
                    elif line.startswith('Ni'):
                        materials[mat_name]['Ni'] = parse_basic_float(line, 'Ni')
                    elif line.startswith('Tr'):
                        materials[mat_name]['Tr'] = parse_basic_float(line, 'Tr')
                    elif line.startswith('Ks'):
                        materials[mat_name]['ks']['albedo'] = parse_basic_float3(line, 'Ks')
                    elif line.startswith('Ke'):
                        materials[mat_name]['ka']['albedo'] = parse_basic_float3(line, 'Ke')
                    elif line.startswith('Kd'):
                        materials[mat_name]['kd']['albedo'] = parse_basic_float3(line, 'Kd')
                    elif line.startswith('map_Ke'):
                        materials[mat_name]['ka']['texture'] = parse_filename(line)
                    elif line.startswith('map_Kd'):
                        materials[mat_name]['kd']['texture'] = parse_filename(line)
                    elif line.startswith('map_Ks'):
                        materials[mat_name]['ks']['texture'] = parse_filename(line)
                    elif line.startswith('newmtl'):
                        break
                    i += 1
            else:
                i += 1
        return materials


def load_obj(mesh_name, filename, out_file) -> Tuple[List[str], Dict[str, str], Dict[str, dict]]:
    out = open(out_file, 'w')
    shapes = set()
    shape_mat = dict()
    materials = dict()
    print('load obj file ' + filename + ' as ' + mesh_name)
    with open(filename, 'r') as f:
        lines = f.readlines()
        part = 0
        shape_base_name = mesh_name
        for line in lines:
            tokens = [x.strip() for x in line.split(' ') if x]
            if tokens[0] == 'mtllib':
                dir = os.path.dirname(filename)
                mtl = load_mtl(os.path.join(dir, tokens[1]))
                materials = {**materials, **mtl}
            if tokens[0] == 'g':
                continue
            if tokens[0] == 'o':
                shape_base_name = mesh_name + '.'.join(tokens[1:])
                part = 0
                continue
            if tokens[0] == 'usemtl':
                mat_name = tokens[1].replace('\\', '/')
                shape_name = '{0}.{1}.{2}'.format(shape_base_name, mat_name, part)
                out.write('o ' + shape_name + '\n')
                if shape_name not in shapes:
                    shapes.add(shape_name)
                shape_mat[shape_name] = mat_name
                out.write(line)
                part += 1
            else:
                out.write(line)
    # print(shapes, shape_mat, materials)
    return (list(shapes), shape_mat, materials)


def add_obj_to_scene(obj_file, scene_file):
    try:
        f = open(scene_file, 'r')
        scene = json.load(f)
        f.close()
    except FileNotFoundError:
        scene = dict()
    if 'objects' not in scene:
        scene['objects'] = []
    if 'materials' not in scene:
        scene['materials'] = dict()
    if 'shapes' not in scene:
        scene['shapes'] = dict()
    mesh_count = len(scene['objects'])
    mesh_name = 'mesh' + str(mesh_count) + '.obj'
    shapes, shape_mat, materials = load_obj(mesh_name, obj_file, mesh_name)
    mesh_mat_name = 'mesh' + str(mesh_count)
    scene['objects'].append({
        "file": mesh_name,
        "name": mesh_mat_name,
        "transform": {
            "rotation": [0, 0, 0],
            "translation": [0, 0, 0],
            "scale": 1
        }
    })
    scene['shapes'][mesh_mat_name] = {**scene['shapes'], **shape_mat}
    scene['materials'] = {**scene['materials'], **materials}
    f = open(scene_file, 'w')
    json.dump(scene, f)


# if __name__ == '__main__' and len(sys.argv) > 1:
#     action = sys.argv[1]
#     src = sys.argv[2]
#     dst = sys.argv[3]
add_obj_to_scene('data/test-scenes/texture/text.obj', 'data/test-scenes/texture/scene.json')
