//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_INTERACTION_H
#define MIYUKI_INTERACTION_H

#include "scene.h"
#include "mesh.h"
#include "reflection.h"
#include "ray.h"

namespace Miyuki {
    struct Material;

    class BSDF;

    class TriangularMesh;

    class MeshInstance;

    struct Primitive;

    struct Interaction {
    public:

        Ref<const Primitive> primitive;
        Ref<const Material> material;
        BSDF *bsdf;
        Vec3f wi, normal, Ng, hitpoint, wo; // world
        Point2f uv;
        int32_t geomID, primID;

        Point2f textureCoord() const;

        void computeScatteringFunctions(MemoryArena &arena);
    };

}
#endif //MIYUKI_INTERACTION_H
