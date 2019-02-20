//
// Created by Shiina Miyuki on 2019/1/15.
//

#include "mesh.h"
#include "scene.h"
#include "texture.h"
#include "../bsdfs/bsdf.h"

using namespace Miyuki;

std::shared_ptr<TextureMapping2D> loadFromPNG(const std::string &filename) {
    std::vector<unsigned char> pixelData;
    uint32_t w, h;
    lodepng::decode(pixelData, w, h, filename);
    if (pixelData.empty()) {
        fmt::print(stderr, "Error loading texture {}\n", filename);
        return std::unique_ptr<TextureMapping2D>(nullptr);
    }
    fmt::print("Loaded texture {}\n", filename);
    return std::make_shared<TextureMapping2D>(TextureMapping2D(pixelData, Point2i(w, h)));
}

std::shared_ptr<TriangularMesh> Miyuki::LoadFromObj(
        BSDFFactory &materialFactory,
        MaterialList *materialList,
        const char *filename,
        TextureOption opt) {
    std::shared_ptr<TriangularMesh> mesh(new TriangularMesh());
    int32_t mOffset = materialList->size();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    bool succ = true;
    std::map<std::string, std::shared_ptr<TextureMapping2D>> textures;
    bool use = (int32_t) opt;
    fmt::print("Use texture = {}\n", use);
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
            std::shared_ptr<TextureMapping2D> kaMap, kdMap, ksMap;
            if (use && !m.diffuse_texname.empty()) {
                if (textures.find(m.diffuse_texname) == textures.end()) {
                    kdMap = loadFromPNG(m.diffuse_texname);
                    textures[m.diffuse_texname] = kdMap;
                } else
                    kdMap = textures[m.diffuse_texname];
            }
            if (use && !m.specular_texname.empty()) {
                if (textures.find(m.specular_texname) == textures.end()) {
                    ksMap = loadFromPNG(m.specular_texname);
                    textures[m.specular_texname] = ksMap;
                } else
                    ksMap = textures[m.specular_texname];
            }
            if (use && !m.emissive_texname.empty()) {
                if (textures.find(m.emissive_texname) == textures.end()) {
                    kaMap = loadFromPNG(m.emissive_texname);
                    textures[m.emissive_texname] = kaMap;
                } else
                    kaMap = textures[m.emissive_texname];
            }
            ColorMap ka(m.emission[0], m.emission[1], m.emission[2], kaMap);
            ColorMap kd(m.diffuse[0], m.diffuse[1], m.diffuse[2], kdMap);
            ColorMap ks(m.specular[0], m.specular[1], m.specular[2], ksMap);
            MaterialInfo materialInfo(ka, kd, ks);
            materialInfo.Ni = m.ior;
            materialInfo.Tr = 1 - m.dissolve;
            materialInfo.roughness = m.roughness;
            materialInfo.parameters = m.unknown_parameter;
            materialInfo.bsdfType = m.bsdfType;
            materialInfo.fresnelType = "default";
            materialInfo.microfacetType = "default";
            materialInfo.Ns = m.shininess;
            if (materialInfo.parameters.find("fresnel") != materialInfo.parameters.end()) {
                materialInfo.fresnelType = materialInfo.parameters["fresnel"];
            }
            if (materialInfo.parameters.find("microfacet") != materialInfo.parameters.end()) {
                materialInfo.microfacetType = materialInfo.parameters["microfacet"];
            }
            materialList->addMaterial(m.name, materialList->size());
            materialList->emplace_back(materialFactory(materialInfo));
        }
        mesh->vertex.reserve(attrib.vertices.size());
        for (auto i = 0; i < attrib.vertices.size(); i += 3) {
            mesh->vertex.emplace_back(Vec3f(attrib.vertices[i],
                                            attrib.vertices[i + 1],
                                            attrib.vertices[i + 2]));
        }
        mesh->normal.reserve(attrib.normals.size());
        for (auto i = 0; i < attrib.normals.size(); i += 3) {
            mesh->normal.emplace_back(Vec3f(attrib.normals[i],
                                            attrib.normals[i + 1],
                                            attrib.normals[i + 2]));
        }
// Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int32_t fv = shapes[s].mesh.num_face_vertices[f];
                assert(fv == 3); // we are using trig mesh
                // Loop over vertices in the face.
                Triangle triangle;
                triangle.useTexture = true;
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    assert(idx.vertex_index < mesh->vertexCount());
                    triangle.vertex[v] = idx.vertex_index;

                    if (idx.normal_index < mesh->normal.size())
                        triangle.normal[v] = idx.normal_index;
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
                    mesh->normal.emplace_back(triangle.trigNorm);
                    for (int32_t i = 0; i < 3; i++) {
                        triangle.normal[i] = mesh->normal.size() - 1;
                    }
                }

                triangle.materialId = shapes[s].mesh.material_ids[f] + mOffset;
                mesh->trigs.emplace_back(triangle);
                index_offset += fv;
            }
        }
        fmt::print("{} vertices, {} normals, {} triangles\n", mesh->vertex.size(), mesh->normal.size(),
                   mesh->trigs.size());
        fmt::print("Done loading {}\n", filename);
    });
    if (succ)
        return mesh;
    return nullptr;
}

MeshInstance::MeshInstance(std::shared_ptr<TriangularMesh> mesh, const Transform &t) {
    primitives.resize(mesh->triangleCount());
    vertices.resize(mesh->vertexCount());
    normals.resize(mesh->normCount());
    parallelFor(0u, mesh->vertexCount(), [&](int i) {
        vertices[i] = t.apply(mesh->vertexArray()[i]);
    });
    parallelFor(0u, mesh->normCount(), [&](int i) {
        normals[i] = t.applyRotation(mesh->normArray()[i]).normalized();
    });
    parallelFor(0u, mesh->triangleCount(), [&](int i) {
        auto &trig = mesh->triangleArray()[i];
        primitives[i].materialId = trig.materialId;
        primitives[i].Ng = trig.trigNorm;
        primitives[i].area = trig.area;
        for (int32_t k = 0; k < 3; k++) {
            primitives[i].normal[k] = &normals[trig.normal[k]];//t.applyRotation(mesh->normal[trig.normal[k]]).normalized();
            primitives[i].vertices[k] = &vertices[trig.vertex[k]];//t.apply(mesh->vertex[trig.vertex[k]]);
            primitives[i].textCoord[k] = trig.textCoord[k];
        }
    });
}

Vec3f Primitive::normalAt(const Point2f &p) const {
    return pointOnTriangle(*normal[0], *normal[1], *normal[2], p.x(), p.y());
}

bool Primitive::intersect(const Ray &ray, Float *tHit, IntersectionInfo *isct) const {
    const Float eps = 0.0000001;
    Vec3f edge1, edge2, h, s, q;
    Float a, f, u, v;
    edge1 = *vertices[1] - *vertices[0];
    edge2 = *vertices[2] - *vertices[0];
    h = Vec3f::cross(ray.d, edge2);
    a = Vec3f::dot(edge1, h);
    if (a > -eps && a < eps)
        return false;    // This ray is parallel to this triangle.
    f = 1.0f / a;
    s = ray.o - *vertices[0];
    u = f * (Vec3f::dot(s, h));
    if (u < 0.0f || u > 1.0f)
        return false;
    q = Vec3f::cross(s, edge1);
    v = f * Vec3f::dot(ray.d, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;
    float t = f * Vec3f::dot(edge2, q);
    if (t > eps) // ray intersection
    {
        *tHit = t;
        isct->hitpoint = ray.o + t * ray.d;
        isct->Ng = Ng;
        isct->normal = pointOnTriangle(*normal[0], *normal[1], *normal[2], u, v);
        return true;
    } else
        return false;
}

Float Primitive::pdf(const IntersectionInfo &ref, const Vec3f &wi) const {
    Ray ray = ref.spawnRay(wi);
    Float tHit;
    IntersectionInfo info;
    if (!intersect(ray, &tHit, &info)) { return 0; }
    return (ref.hitpoint - info.hitpoint).lengthSquared() / (Vec3f::absDot(info.Ng, -1 * wi) * area);
}
