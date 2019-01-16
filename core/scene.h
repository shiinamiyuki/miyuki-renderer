//
// Created by xiaoc on 2019/1/12.
//

#ifndef MIYUKI_SCENE_HPP
#define MIYUKI_SCENE_HPP

#include "util.h"
#include "Film.h"

namespace Miyuki {
    struct Material{
        Vec3f ka, kd, ks;
        Float glossiness;
    };
    class MaterialList :
            public std::vector<Material>{
        std::unordered_map<std::string, int> map;
    public:

    };
    class Scene {
        RTCScene rtcScene;
        Film film;
        MaterialList materialList;
        void commit();

    public:
        void loadObj(const char *filename);

        Scene();

        ~Scene();

        RTCScene getRTCSceneHandle() const { return rtcScene; }
    };
}


#endif //MIYUKI_SCENE_HPP
