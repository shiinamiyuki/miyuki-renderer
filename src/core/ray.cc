//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "ray.h"
#include "mesh.h"
#include "materials/material.h"

namespace Miyuki {

    Intersection::Intersection(const Ray &ray) {
        rayHit.ray = ray.toRTCRay();
        rayHit.ray.flags = 0;
        rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        rayHit.hit.primID = RTC_INVALID_GEOMETRY_ID;
        rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
        rtcInitIntersectContext(&context);
    }

    bool Intersection::hit() const {
        return primId != RTC_INVALID_GEOMETRY_ID && geomId != RTC_INVALID_GEOMETRY_ID;
    }

    bool Intersection::intersect(RTCScene scene) {
        rtcIntersect1(scene, &context, &rayHit);
        primId = rayHit.hit.primID;
        geomId = rayHit.hit.geomID;
        return hit();
    }

    Spectrum Intersection::Le(const Vec3f &wi) const {
        if(Vec3f::dot(wi, Ns) > 0)
            return primitive->material()->emission.albedo;
        return {};
    }

    RTCRay Ray::toRTCRay() const {
        RTCRay ray;
        ray.dir_x = d.x();
        ray.dir_y = d.y();
        ray.dir_z = d.z();
        ray.org_x = o.x();
        ray.org_y = o.y();
        ray.org_z = o.z();
        ray.tnear = near;
        ray.tfar = far;
        ray.flags = 0;
        return ray;
    }

    void RayDifferential::scaleDifferentials(Float s) {
        rxOrigin = o + (rxOrigin - o) * s;
        ryOrigin = o + (ryOrigin - o) * s;
        rxDirection = d + (rxDirection - d) * s;
        ryDirection = d + (ryDirection - d) * s;
    }
}