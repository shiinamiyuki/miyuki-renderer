//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_EMBREESCENE_H
#define MIYUKI_EMBREESCENE_H

#include "miyuki.h"
#include "mesh.h"
#include "ray.h"

namespace Miyuki {
    RTCDevice GetEmbreeDevice();

    class Scene;

    class EmbreeScene {
        friend class Scene;

        RTCScene scene;

        void commit();

    public:
        EmbreeScene();

        void addMesh(std::shared_ptr<Mesh> mesh, int id);
        RTCScene getRTCScene(){
            return scene;
        }
        ~EmbreeScene();
    };
}
#endif //MIYUKI_EMBREESCENE_H
