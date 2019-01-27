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
        Float scale;

        Transform();

        Transform(const Vec3f &t, const Vec3f &r, Float s);

        //rotation then translation
        Vec3f apply(const Vec3f &) const;
    };
}
#endif //MIYUKI_TRANSFORM_H
