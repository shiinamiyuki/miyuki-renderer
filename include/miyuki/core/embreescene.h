//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_EMBREESCENE_H
#define MIYUKI_EMBREESCENE_H

#include "miyuki.h"
#include "mesh.h"
#include <core/ray.h>
#include <core/intersection.hpp>
#include <core/accelerators/accelerator.h>
#include <embree3/rtcore.h>

namespace Miyuki {
    RTCDevice GetEmbreeDevice();

    class Scene;

    class EmbreeScene : public Accelerator {
        friend class Scene;

        RTCScene scene;

        void commit();

    public:
        EmbreeScene();

        void addMesh(std::shared_ptr<Mesh> mesh, int id);

		bool intersect(const Ray & ray, Intersection * isct);

        RTCScene getRTCScene(){
            return scene;
        }
        ~EmbreeScene();
    };
}
#endif //MIYUKI_EMBREESCENE_H
