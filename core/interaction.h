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
    private:
        Vec3f localX, localZ;
    public:
        void computeLocalCoordinate();

        Ref<const Primitive> primitive;
        Ref<const Material> material;
        BSDF *bsdf;
        Vec3f wi, normal, Ng, hitpoint, wo; // world
        Point2f uv;
        int geomID, primID;
        Vec3f localWi, localWo;// local
        Vec3f worldToLocal(const Vec3f &v) const; // transform according to shading normal

        Vec3f localToWorld(const Vec3f &v) const;

        Point2f textureCoord() const;
        Ray spawnWi()const;
    };

}
#endif //MIYUKI_INTERACTION_H
