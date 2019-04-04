//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_RAY_H
#define MIYUKI_RAY_H

#include "miyuki.h"
#include "geometry.h"
#include "spectrum.h"

namespace Miyuki {
    struct Ray {
        Vec3f o, d;
        mutable Float near, far;
        int excludePrimId = -1, excludeGeomId = -1;
        /*unused*/
        Float time;

        Ray() : time(0), near(0), far(INF) {}

        Ray(const Vec3f &o, const Vec3f &d, Float near = EPS, Float far = INF) : o(o), d(d), near(near), far(far) {}

        RTCRay toRTCRay() const;

        Vec3f operator()(Float t) {
            return o + t * d;
        }
    };

    struct RayDifferential : Ray {
        Vec3f rxOrigin, ryOrigin;
        Vec3f rxDirection, ryDirection;
        bool hasDifferential;

        RayDifferential() { hasDifferential = false; }

        RayDifferential(const Ray &ray) : Ray(ray) { hasDifferential = false; }

        RayDifferential(const Vec3f &o, const Vec3f &d, Float near = EPS, Float far = INF)
                : Ray(o, d, near, far) { hasDifferential = false; }

        // scale differentials according to estimated spacing s
        void scaleDifferentials(Float s) {
            rxOrigin = o + (rxOrigin - o) * s;
            ryOrigin = o + (ryOrigin - o) * s;
            rxDirection = d + (rxDirection - d) * s;
            ryDirection = d + (ryDirection - d) * s;
        }

    };

    struct Primitive;

    struct Intersection {
        RTCIntersectContext context;
        RTCRayHit rayHit;
        Ray ray;
        Vec3f p;
        Vec3f Ng;
        Vec3f Ns;
        Vec3f wo;
        int32_t primId = -1, geomId = -1;
        const Primitive *primitive = nullptr;
        Point2f uv;
        Vec3f dpdu, dpdv;
        int excludePrimId = -1, excludeGeomId = -1;
        Intersection(const Ray &ray = Ray());

        bool hit() const;

        bool intersect(RTCScene scene);

        Float hitDistance() const {
            return rayHit.ray.tfar;
        }

        Spectrum Le(const Vec3f &) const;
    };
}
#endif //MIYUKI_RAY_H
