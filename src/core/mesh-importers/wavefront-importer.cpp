// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "wavefront-importer.h"
#include <miyuki.renderer/graph.h>
#include <miyuki.foundation/log.hpp>
#include <miyuki.renderer/mesh.h>
#include <fstream>
#include <optional>
#include <tiny_obj_loader.h>
#include <miyuki.renderer/mesh-importer.h>

#include "../bsdfs/diffusebsdf.h"
#include "../bsdfs/microfacet.h"
#include "../bsdfs/mixbsdf.h"
#include "../shaders/common-shader.h"

namespace miyuki::core {
    void split(const std::string &src, const char delim, std::vector<std::string> &result) {
        std::string s;
        for (size_t i = 0; i < src.length();) {
            if (src[i] == delim) {
                result.push_back(s);
                s.clear();
                while (i < src.length() && src[i] == delim) {
                    i++;
                }
            } else {
                s += src[i];
                i++;
            }
        }
        if (!s.empty()) {
            result.push_back(s);
        }
    }

    std::shared_ptr<Material> convertFromMTL(const tinyobj::material_t &mat) {
        auto material = std::make_shared<Material>();
        auto kd = Vec3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        auto ks = Vec3f(mat.specular[0], mat.specular[1], mat.specular[2]);
        auto emission = Vec3f(mat.emission[0], mat.emission[1], mat.emission[2]);
        auto strength = maxComp(emission) == 0.0 ? 0.0 : maxComp(emission);
        emission = strength == 0.0 ? vec3(0) : emission / maxComp(emission);
        auto diffuse = std::make_shared<DiffuseBSDF>(std::make_shared<RGBShader>(kd));
        auto specular = std::make_shared<DiffuseBSDF>(std::make_shared<RGBShader>(ks));
        auto mixed = std::make_shared<MixBSDF>(std::make_shared<FloatShader>(0.5f), diffuse, specular);
        material->bsdf = mixed;
        material->emission = std::make_shared<RGBShader>(emission);
        material->emissionStrength = std::make_shared<FloatShader>(strength);
        return material;
    }

    MeshImportResult WavefrontImporter::importMesh(const fs::path &path) {
        log::log("Importing {}\n", path.string());
        fs::path parent_path = fs::absolute(path).parent_path();
        fs::path file = path.filename();
        CurrentPathGuard _guard;
        if (!parent_path.empty())
            fs::current_path(parent_path);

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        std::string _file = file.string();
        MeshImportResult result;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, _file.c_str());
        if (!ret) {
            log::log("error loading {}\n", path.string());
            return result;
        }
        //log::log("vert: {}\n",attrib.vertices.size());
        auto mesh = std::make_shared<Mesh>();
        mesh->filename = path.string();
        mesh->_vertex_data.position.reserve(attrib.vertices.size());
        for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
            mesh->_vertex_data.position.emplace_back(
                    Vec3f(attrib.vertices[i + 0], attrib.vertices[i + 1], attrib.vertices[i + 2]));
        }

        mesh->_vertex_data.normal.resize(mesh->_vertex_data.position.size());
        mesh->_vertex_data.tex_coord.resize(mesh->_vertex_data.position.size());

        std::unordered_map<std::string, size_t> name_to_id;
        std::unordered_map<size_t, std::string> id_to_name;
        std::unordered_map<std::string, std::unordered_map<int, std::string>> to_mangled_name;

        for (size_t i = 0; i < shapes.size(); i++) {
            const auto &name = shapes[i].name;

            auto iter = to_mangled_name.find(name);

            if (iter == to_mangled_name.end()) {
                to_mangled_name[name] = {};
            }
            auto &m = to_mangled_name[name];
            size_t count = 0;
            for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                bool new_name = false;
                std::string final_name;

                auto mat_id = shapes[i].mesh.material_ids[f];
                if (m.find(mat_id) == m.end() || mat_id == -1) {
                    new_name = true;
                    if (mat_id >= 0) {
                        final_name = fmt::format("{}:{}", name, materials[mat_id].name);
                    } else {
                        final_name = fmt::format("{}:part{}", name, count + 1);
                    }
                }
                if (new_name) {
                    count++;
                    m[mat_id] = final_name;
                    name_to_id[final_name] = name_to_id.size();
                    auto mat = convertFromMTL(materials[mat_id]);
                    result.materials.emplace_back(mat);
                    mesh->materials[final_name] = mat;
                    id_to_name[name_to_id[final_name]] = final_name;
                    fmt::print("generated {}\n", final_name);
                }
            }
        }
        for (size_t i = 0; i < name_to_id.size(); i++) {
            mesh->_names.emplace_back(id_to_name[i]);
        }

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                MeshTriangle primitive;
                auto mat_id = shapes[s].mesh.material_ids[f];
                primitive.name_id = name_to_id.at(to_mangled_name.at(shapes[s].name).at(mat_id));
                int fv = shapes[s].mesh.num_face_vertices[f];
                Point3i vertex_indices;
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    vertex_indices[v] = idx.vertex_index;
                }
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    if (idx.normal_index >= 0) {
                        mesh->_vertex_data.normal[idx.vertex_index] =
                                Vec3f(attrib.normals[3 * idx.normal_index + 0],
                                      attrib.normals[3 * idx.normal_index + 1],
                                      attrib.normals[3 * idx.normal_index + 2]);
                    } else {
                        // use the last normal (Ng)

                        Vec3f e1 = mesh->_vertex_data.position[vertex_indices[2]] -
                                   mesh->_vertex_data.position[vertex_indices[0]];
                        Vec3f e2 = mesh->_vertex_data.position[vertex_indices[1]] -
                                   mesh->_vertex_data.position[vertex_indices[0]];
                        mesh->_vertex_data.normal[idx.vertex_index] = normalize(cross(e1, e2));
                    }
                    if (idx.texcoord_index >= 0) {
                        mesh->_vertex_data.tex_coord[idx.vertex_index] = Point2f(
                                attrib.texcoords[2 * idx.texcoord_index + 0],
                                attrib.texcoords[2 * idx.texcoord_index + 1]);
                    } else {
                        mesh->_vertex_data.tex_coord[idx.vertex_index] = Point2f(v > 0, v > 1);
                    }
                }
                index_offset += fv;

                // per-face material
                // shapes[s].mesh.material_ids[f];

                primitive.mesh = mesh.get();
                mesh->triangles.push_back(primitive);
                mesh->_indices.push_back(vertex_indices);
            }
        }
        log::log("loaded {} vertices, {} normals, {} primitives\n", mesh->_vertex_data.position.size(),
                 mesh->_vertex_data.normal.size(), mesh->triangles.size());
        mesh->_loaded = true;
        result.mesh = mesh;
        return result;
    }
} // namespace miyuki::core