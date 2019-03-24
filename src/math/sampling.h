//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_SAMPLING_H
#define MIYUKI_SAMPLING_H

#include "miyuki.h"
#include "core/geometry.h"

namespace Miyuki {
    Point2f ConcentricSampleDisk(const Point2f &u);

    inline Vec3f CosineWeightedHemisphereSampling(const Point2f &u) {
        Float theta = u[0] * 2 * PI;
        Float r = sqrtf(u[1]);
        return Vec3f(sin(theta) * r, sqrt(1 - r * r), cos(theta) * r);
    }

    inline Point2f InvertCosineWeightedHemisphereSampling(const Vec3f &v) {
        // makes CLion happy
        auto y = v.x();
        auto x = v.z();
        auto r = x * x + y * y;
        Float theta = std::atan2(y, x);
        if(theta < 0){
            theta += PI;
        }
        return {theta / (2 * PI), r};
    }

    inline Vec3f UniformTriangleSampling(const Point2f &u0, const Vec3f &v0, const Vec3f &v1, const Vec3f &v2) {
        Float u = u0[0], v = u0[1];
        if (u + v >= 1) {
            u = 1 - u;
            v = 1 - v;
        }
        return PointOnTriangle(v0, v1, v2, u, v);
    }

    inline Vec3f UniformSampleSphere(const Point2f &u) {
        Float z = 1 - 2 * u[0];
        Float r = std::sqrt(std::max((Float) 0, (Float) 1 - z * z));
        Float phi = 2 * PI * u[1];
        return Vec3f(r * std::cos(phi), r * std::sin(phi), z);
    }

    inline Float UniformSpherePdf() {
        return 1.0f / (PI * 4);
    }
}

#endif //MIYUKI_SAMPLING_H
