//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_MESH_H
#define MIYUKI_MESH_H

#include "miyuki.h"
#include "math/transform.h"
#include "core/geometry.h"

namespace Miyuki {
    class Light;

    struct Mesh;

    class Material;

    struct Ray;
    struct Intersection;

    struct Primitive {
        uint32_t vertices[3];
        uint32_t normals[3];
        Point2f textureCoord[3];
        Vec3f Ng;
        Mesh *instance;
        int32_t nameId;
        Float area;

        Primitive();

        const Vec3f& v(int32_t) const;

        const Vec3f& n(int32_t) const;

        Vec3f Ns(const Point2f &uv) const;

        Material *material() const;

        const std::string &name() const;

        bool intersect(const Ray &, Intersection *) const;

        Light * light()const;

        void setLight(Light * light);
    };


    struct Mesh {
        std::vector<Vec3f> vertices, normals;
        std::vector<Primitive> primitives;
        std::vector<std::string> names;
        std::vector<std::shared_ptr<Material>> materials;
        Mesh(const std::string &filename);
        std::shared_ptr<Mesh> instantiate(const Transform &transform = Transform()) const;
        std::unordered_map<const Primitive *, Light *> lightMap;
    };
}
#endif //MIYUKI_MESH_H
