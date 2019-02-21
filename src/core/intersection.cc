//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "intersection.h"
#include "scene.h"
#include "../bsdfs/bsdf.h"
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

Point2f IntersectionInfo::textureCoord() const {
    return pointOnTriangle(primitive->textCoord[0], primitive->textCoord[1], primitive->textCoord[2], uv.x(), uv.y());
}

Spectrum IntersectionInfo::Le(const Vec3f &w) const {
    if(Vec3f::dot(w, normal) > 0)
        return bsdf->Ka().color;
    return {};
}
