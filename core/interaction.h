//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_INTERACTION_H
#define MIYUKI_INTERACTION_H

#include "scene.h"
#include "mesh.h"

namespace Miyuki {
    struct Interaction {
        using Primitive = Mesh::MeshInstance::Primitive;
        Ref<const Primitive> primitive;
        Vec3f wi, norm, hitpoint;
        Point2f uv;
        int geomID, primID;
    };

}
#endif //MIYUKI_INTERACTION_H
