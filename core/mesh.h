//
// Created by Shiina Miyuki on 2019/1/15.
//

#ifndef MIYUKI_MESH_H
#define MIYUKI_MESH_H

#include "geometry.h"
#include "spectrum.h"
#include "transform.h"
#include "material.h"

namespace Miyuki {
    class MaterialList;


    struct Triangle {
        int32_t vertex[3];
        int32_t normal[3];
        int32_t materialId;
        bool useNorm;
        Vec3f trigNorm;
        Point2f textCoord[3];
        Float area;
        bool useTexture;

        Triangle() = default;
    };

    class TriangularMesh;

    struct MeshInstance;
    enum class TextureOption;

    class MaterialFactory;

    std::shared_ptr<TriangularMesh>
    LoadFromObj(MaterialFactory &, MaterialList *materialList, const char *filename, TextureOption opt);

    class TriangularMesh {
        std::vector<Vec3f> vertex, normal;
        std::vector<Triangle> trigs;
    public:
        friend struct MeshInstance;

        TriangularMesh() {}

        friend std::shared_ptr<TriangularMesh> LoadFromObj(
                MaterialFactory &,
                MaterialList *materialList,
                const char *filename, TextureOption opt);

        const Triangle *triangleArray() const { return trigs.data(); }

        const Vec3f *vertexArray() const { return vertex.data(); }

        const Vec3f *normArray() const { return normal.data(); }

        size_t triangleCount() const { return trigs.size(); }

        size_t vertexCount() const { return vertex.size(); }

        size_t normCount() const { return normal.size(); }
    };
    struct Ray;
    struct Interaction;
    struct Primitive {
        Vec3f normal[3], vertices[3];
        Point2f textCoord[3];
        Vec3f Ng;
        int32_t materialId;
        Float area;
        Primitive() {}

        Vec3f normalAt(const Point2f &) const;

        bool intersect(const Ray &ray, Float *tHit, Interaction *) const;
        Float pdf(const Interaction&ref, const Vec3f & wi)const;
    };

    struct MeshInstance {


        std::vector<Primitive> primitives;

        const Primitive &getPrimitive(uint32_t primID) const { return primitives[primID]; }

        MeshInstance() = default;

        MeshInstance(std::shared_ptr<TriangularMesh>, const Transform &t = Transform());
    };
}
#endif //MIYUKI_MESH_H
