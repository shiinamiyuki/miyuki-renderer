//
// Created by xiaoc on 2019/1/15.
//

#include "mesh.h"

using namespace Miyuki;
using Mesh::TriangularMesh;

std::shared_ptr<TriangularMesh> Miyuki::Mesh::LoadFromObj(MaterialList *materialList, const char *filename) {
    std::shared_ptr<TriangularMesh> mesh(new TriangularMesh());
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    cxx::filesystem::path currentPath = cxx::filesystem::current_path();

    cxx::filesystem::path inputFile(filename);
    auto file = inputFile.filename().string();
    auto parent = inputFile.parent_path();
    cxx::filesystem::current_path(parent);

    std::string err;
    fmt::print("Loading OBJ file {}\n", filename);
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str());
    cxx::filesystem::current_path(currentPath);
    if (!err.empty()) { // `err` may contain warning message.
        fmt::print(stderr, "{}\n", err);
    }
    if (!ret) {
        fmt::print(stderr, "error loading OBJ file {}\n",file);
        return nullptr;
    }
    for (const auto &m :materials) {
        Material material;
        for (int i = 0; i < 3; i++) {
            material.ka[i] = m.emission[i];
            material.kd[i] = m.diffuse[i];
            material.ks[i] = m.specular[i];
        }
        materialList->emplace_back(material);
    }
    for (auto i = 0; i < attrib.vertices.size(); i += 3) {
        mesh->vertex.emplace_back(Vec3f(attrib.vertices[3 * i],
                                        attrib.vertices[3 * i + 1],
                                        attrib.vertices[3 * i + 2]));
    }
    for (auto i = 0; i < attrib.normals.size(); i += 3) {
        mesh->norm.emplace_back(Vec3f(attrib.normals[3 * i],
                                      attrib.normals[3 * i + 1],
                                      attrib.normals[3 * i + 2]));
    }
// Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            assert(fv == 3); // we are using trig mesh
            // Loop over vertices in the face.
            Triangle triangle;
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                triangle.vertex[v] = idx.vertex_index;
                if (idx.normal_index < mesh->norm.size())
                    triangle.norm[v] = idx.normal_index;
                else {
                    triangle.useNorm = false;
                }
//                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
//                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
//                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
//                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
//                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
//                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
//                tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
//                tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                // Optional: vertex colors
                // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
                // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
                // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
            }
            triangle.trigNorm = Vec3f::cross(mesh->vertex[triangle.vertex[1]] - mesh->vertex[triangle.vertex[0]],
                                             mesh->vertex[triangle.vertex[2]] - mesh->vertex[triangle.vertex[0]]);
            triangle.area = triangle.trigNorm.length() / 2;
            triangle.trigNorm.normalize();
            triangle.materialId = shapes[s].mesh.material_ids[f];
            mesh->trigs.emplace_back(triangle);
            index_offset += fv;
        }
    }
    fmt::print("{} vertices, {} normals, {} triangles\n", mesh->vertex.size(), mesh->norm.size(), mesh->trigs.size());
    fmt::print("Done loading {}\n",filename);
    return mesh;
}

