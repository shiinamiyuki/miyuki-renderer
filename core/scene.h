//
// Created by xiaoc on 2019/1/12.
//

#ifndef MIYUKI_SCENE_HPP
#define MIYUKI_SCENE_HPP

#include "util.h"

namespace Miyuki {
    class Scene {
        RTCScene rtcScene;

        void commit();

    public:
        Scene();

        ~Scene();

        RTCScene getRTCSceneHandle() const { return rtcScene; }
    };
}


#endif //MIYUKI_SCENE_HPP
