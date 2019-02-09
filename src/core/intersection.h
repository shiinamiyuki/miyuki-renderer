//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_INTERSECTION_H
#define MIYUKI_INTERSECTION_H

#include "../utils/util.h"
#include "ray.h"
#include "spectrum.h"

namespace Miyuki {
    class Scene;

    class Intersection {
    public:
        RTCIntersectContext context;
        RTCRayHit rayHit;

        Intersection(const RTCRay &ray);

        Intersection(const Ray &ray);

        void intersect(const Scene &scene);

        void occlude(RTCScene scene);

        bool hit() const { return rayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID; }

        uint32_t geomID() const { return rayHit.hit.geomID; }

        uint32_t primID() const { return rayHit.hit.primID; }

        Float hitDistance() const { return rayHit.ray.tfar; }

        Vec3f intersectionPoint() const;
    };

    struct Primitive;

    class BSDF;

    struct IntersectionInfo {
    public:

        Ref<const Primitive> primitive;
        BSDF *bsdf;
        Vec3f wi, normal, Ng, hitpoint, wo; // world
        Point2f uv;
        int32_t geomID, primID;
        Float distance;
        Point2f textureCoord() const;

        Ray spawnRay(const Vec3f &w) const {
            return {hitpoint, w};
        }
        Spectrum Le(const Vec3f&w)const;
    };
}
#endif //MIYUKI_INTERSECTION_H
