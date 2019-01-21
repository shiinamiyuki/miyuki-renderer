//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_TRANSFORM_H
#define MIYUKI_TRANSFORM_H

#include "util.h"
#include "geometry.h"

namespace Miyuki {
    struct Transform {
        Vec3f translation;
        Vec3f rotation;
        Transform();
        Transform(const Vec3f&t,const Vec3f&r);
        //rotation then translation
        Vec3f apply(const Vec3f&);
    };
}
#endif //MIYUKI_TRANSFORM_H
