//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_INTERACTION_H
#define MIYUKI_INTERACTION_H

#include "scene.h"
#include "mesh.h"
#include "material.h"

namespace Miyuki {
    struct Material;
    struct Interaction {
        using Primitive = Mesh::MeshInstance::Primitive;
        Ref<const Primitive> primitive;
        Ref<const Material> material;
        Vec3f wi, normal, Ng, hitpoint;
        Point2f uv;
        int geomID, primID;
    };

    class SurfaceInteraction : public Interaction {
        BSDF *bsdf;
    public:
        SurfaceInteraction();
    };

}
#endif //MIYUKI_INTERACTION_H
