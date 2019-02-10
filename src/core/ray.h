//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_RAY_H
#define MIYUKI_RAY_H

#include "../utils/util.h"
#include "../math/geometry.h"

namespace Miyuki {
    struct Ray {
        Vec3f o, d;

        Ray(const Vec3f &_o, const Vec3f &_d) : o(_o), d(_d) {}

        Ray(const RTCRay &ray);

        RTCRay toRTCRay() const;
    };

    struct RayDifferential : public Ray {
        RayDifferential(const Vec3f &_o, const Vec3f &_d) : Ray(_o, _d) {}

        RayDifferential(const RTCRay &ray) : Ray(ray) {}
    };
}
#endif //MIYUKI_RAY_H
