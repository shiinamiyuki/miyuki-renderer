//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_RAY_H
#define MIYUKI_RAY_H

#include "../utils/util.h"
#include "../math/geometry.h"
namespace  Miyuki{
    struct Ray {
        Vec3f o, d;

        Ray(const Vec3f &_o, const Vec3f &_d) : o(_o), d(_d) {}

        Ray(const RTCRay &ray);

        RTCRay toRTCRay() const;
    };
}
#endif //MIYUKI_RAY_H
