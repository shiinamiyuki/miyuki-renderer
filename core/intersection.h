//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_INTERSECTION_H
#define MIYUKI_INTERSECTION_H

#include "util.h"
#include "scene.h"
#include "ray.h"
namespace Miyuki {
    struct Intersection {
        RTCIntersectContext context;
        RTCRayHit rayHit;

        Intersection(const RTCRay &ray);

        Intersection(const Ray &ray);

        void intersect(RTCScene scene);

        void occlude(RTCScene scene);

        bool hit() const { return rayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID; }

        unsigned int geomID() const { return rayHit.hit.geomID; }

        unsigned int primID() const { return rayHit.hit.primID; }

        Float hitDistance() const { return rayHit.ray.tfar; }

        Vec3f intersectionPoint() const;
    };
}
#endif //MIYUKI_INTERSECTION_H
