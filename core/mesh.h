//
// Created by Shiina Miyuki on 2019/1/15.
//

#ifndef MIYUKI_MESH_H
#define MIYUKI_MESH_H

#include "geometry.h"
#include "spectrum.h"

namespace Miyuki {
    class MaterialList;
    namespace Mesh {
        struct Triangle {
            int vertex[3];
            int norm[3];
            int materialId;
            bool useNorm;
            Vec3f trigNorm;
            Float area;

            Triangle() = default;
        };

        class TriangularMesh;

        struct MeshInstance;

        std::shared_ptr<TriangularMesh> LoadFromObj(MaterialList *materialList, const char *filename);

        class TriangularMesh {
            std::vector<Vec3f> vertex, norm;
            std::vector<Triangle> trigs;
        public:
            friend struct MeshInstance;

            TriangularMesh() {}

            friend std::shared_ptr<TriangularMesh> LoadFromObj(MaterialList *materialList, const char *filename);

            const Triangle *triangleArray() const { return trigs.data(); }

            const Vec3f *vertexArray() const { return vertex.data(); }

            const Vec3f *normArray() const { return norm.data(); }

            size_t triangleCount() const { return trigs.size(); }

            size_t vertexCount() const { return vertex.size(); }

            size_t normCount() const { return norm.size(); }
        };

        struct MeshInstance {
            enum TextureFlag {
                none = 0,
                ka = 1,
                kd = 2,
                ks = 4,
                all = ka | kd | ks
            };

            struct Primitive {
                Vec3f normal[3], vertices[3];
                Point2i kaTextCoord[3];
                Point2i kdTextCoord[3];
                Point2i ksTextCoord[3];
                int materialId;
                TextureFlag textureFlag;
                Primitive() :textureFlag(none){}

                Vec3f normalAt(const Point2f &) const;
            };

            std::vector<Primitive> primitives;

            const Primitive &getPrimitive(unsigned int primID) const { return primitives[primID]; }

            MeshInstance() = default;

            MeshInstance(std::shared_ptr<TriangularMesh>);
        };
    }
}
#endif //MIYUKI_MESH_H
