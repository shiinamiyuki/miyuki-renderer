//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "intersection.h"
using namespace Miyuki;

Intersection::Intersection(const RTCRay &ray) {
    rayHit.ray = ray;
    rayHit.ray.flags = 0;
    rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    rtcInitIntersectContext(&context);
}

Intersection::Intersection(const Ray &ray) {
    rayHit.ray = ray.toRTCRay();
    rayHit.ray.flags = 0;
    rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    rtcInitIntersectContext(&context);
}

void Intersection::intersect(const Scene& scene) {
    rtcIntersect1(scene.sceneHandle(), &context, &rayHit);
}

void Intersection::occlude(RTCScene scene) {
    rtcOccluded1(scene, &context, &rayHit.ray);
}