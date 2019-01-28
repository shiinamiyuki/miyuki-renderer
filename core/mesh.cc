//
// Created by Shiina Miyuki on 2019/1/15.
//

#include "mesh.h"
#include "scene.h"
#include "texture.h"

using namespace Miyuki;
using Mesh::TriangularMesh;

std::shared_ptr<TextureMapping2D> loadFromPNG(const std::string &filename) {
    std::vector<unsigned char> pixelData;
    unsigned int w, h;
    lodepng::decode(pixelData, w, h, filename);
    if (pixelData.empty()) {
        fmt::print(stderr, "Error loading texture {}\n", filename);
        return std::unique_ptr<TextureMapping2D>(nullptr);
    }
    fmt::print("Loaded texture {}\n", filename);
    return std::make_shared<TextureMapping2D>(TextureMapping2D(pixelData, Point2i(w, h)));
}

std::shared_ptr<TriangularMesh> Miyuki::Mesh::LoadFromObj(MaterialList *materialList, const char *filename) {
    std::shared_ptr<TriangularMesh> mesh(new TriangularMesh());
    int mOffset = materialList->size();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    bool succ = true;
    std::map<std::string, std::shared_ptr<TextureMapping2D>> textures;
    readUnderPath(filename, [&](const std::string &file) -> void {
        std::string err;
        fmt::print("Loading OBJ file {}\n", filename);
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str());

        if (!err.empty()) { // `err` may contain warning message.
            fmt::print(stderr, "{}\n", err);
        }
        if (!ret) {
            fmt::print(stderr, "error loading OBJ file {}\n", file);
            succ = false;
            return;
        }
        for (const auto &m :materials) {
            Material material;
            for (int i = 0; i < 3; i++) {
                material.ka[i] = m.emission[i];
                material.kd[i] = m.diffuse[i];
                material.ks[i] = m.specular[i];
                material.glossiness = (Float) pow(m.roughness, 2);
                material.ior = m.ior;
                material.tr = 1 - m.dissolve;
            }
            if (!m.diffuse_texname.empty()) {
                if (textures.find(m.diffuse_texname) == textures.end()) {
                    material.kdMap = loadFromPNG(m.diffuse_texname);
                    textures[m.diffuse_texname] = material.kdMap;
                } else
                    material.kdMap = textures[m.diffuse_texname];
            }
            if (!m.specular_texname.empty()) {
                if (textures.find(m.specular_texname) == textures.end()) {
                    material.ksMap = loadFromPNG(m.specular_texname);
                    textures[m.specular_texname] = material.ksMap;
                } else
                    material.ksMap = textures[m.specular_texname];
            }
            if (!m.emissive_texname.empty()) {
                if (textures.find(m.emissive_texname) == textures.end()) {
                    material.kaMap = loadFromPNG(m.emissive_texname);
                    textures[m.emissive_texname] = material.kaMap;
                } else
                    material.kaMap = textures[m.emissive_texname];
            }
            materialList->emplace_back(std::move(material));
        }
        for (auto i = 0; i < attrib.vertices.size(); i += 3) {
            mesh->vertex.emplace_back(Vec3f(attrib.vertices[i],
                                            attrib.vertices[i + 1],
                                            attrib.vertices[i + 2]));
        }
        for (auto i = 0; i < attrib.normals.size(); i += 3) {
            mesh->norm.emplace_back(Vec3f(attrib.normals[i],
                                          attrib.normals[i + 1],
                                          attrib.normals[i + 2]));
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
                triangle.useTexture = true;
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    assert(idx.vertex_index < mesh->vertexCount());
                    triangle.vertex[v] = idx.vertex_index;

                    if (idx.normal_index < mesh->norm.size())
                        triangle.norm[v] = idx.normal_index;
                    else {
                        triangle.useNorm = false;

                    }
                    if (2 * idx.texcoord_index < attrib.texcoords.size()) {
                        tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                        tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                        triangle.textCoord[v] = Point2f(tx, ty);
                    } else {
                        triangle.useTexture = false;
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
                if (!triangle.useNorm) {
                    mesh->norm.emplace_back(triangle.trigNorm);
                    for (int i = 0; i < 3; i++) {
                        triangle.norm[i] = mesh->norm.size() - 1;
                    }
                }

                triangle.materialId = shapes[s].mesh.material_ids[f] + mOffset;
                mesh->trigs.emplace_back(triangle);
                index_offset += fv;
            }
        }
        fmt::print("{} vertices, {} normals, {} triangles\n", mesh->vertex.size(), mesh->norm.size(),
                   mesh->trigs.size());
        fmt::print("Done loading {}\n", filename);
    });
    if (succ)
        return mesh;
    return nullptr;
}

Mesh::MeshInstance::MeshInstance(std::shared_ptr<TriangularMesh> mesh) {
    primitives.resize(mesh->triangleCount());
    for (int i = 0; i < mesh->triangleCount(); i++) {
        auto &trig = mesh->triangleArray()[i];
        primitives[i].materialId = trig.materialId;
        for (int k = 0; k < 3; k++) {
            primitives[i].normal[k] = mesh->norm[trig.norm[k]];
            primitives[i].vertices[k] = mesh->vertex[trig.vertex[k]];
            primitives[i].textCoord[k] = trig.textCoord[k];
        }
    }
}

Vec3f Mesh::MeshInstance::Primitive::normalAt(const Point2f &p) const {
    return pointOnTriangle(normal[0], normal[1], normal[2], p.x(), p.y());
}
