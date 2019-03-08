//
// Created by Shiina Miyuki on 2019/3/6.
//
#include "light.h"
#include "core/scene.h"

namespace Miyuki {

    bool VisibilityTester::visible(Scene &scene) {
        Intersection intersection;
        if (!scene.intersect(shadowRay, &intersection)) {
            return true;
        }
        return intersection.geomId == geomId && intersection.primId == primId;
    }
}
