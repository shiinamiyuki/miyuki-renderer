//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "ray.h"
#include "mesh.h"
#include "materials/material.h"

namespace Miyuki {

    Intersection::Intersection(const Ray &ray) {
        this->ray = ray;
        rayHit.ray = ray.toRTCRay();
        rayHit.ray.flags = 0;
        rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        rayHit.hit.primID = RTC_INVALID_GEOMETRY_ID;
        rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
        excludeGeomId = ray.excludeGeomId;
        excludePrimId = ray.excludePrimId;
        rtcInitIntersectContext(&context);
    }

    bool Intersection::hit() const {
        return primId != RTC_INVALID_GEOMETRY_ID && geomId != RTC_INVALID_GEOMETRY_ID;
    }

    bool Intersection::intersect(RTCScene scene) {
        for (int i = 0; i < 8; i++) {
            rtcIntersect1(scene, &context, &rayHit);
            if (rayHit.hit.primID == RTC_INVALID_GEOMETRY_ID || rayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
                return false;
            if (excludePrimId != rayHit.hit.primID || excludeGeomId != rayHit.hit.geomID) {
                break;
            } else {
                ray.o += (EPS + hitDistance()) * ray.d;
                ray.far -= (EPS + hitDistance());
                rayHit.ray = ray.toRTCRay();
                rayHit.ray.flags = 0;
                rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
                rayHit.hit.primID = RTC_INVALID_GEOMETRY_ID;
                rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
                rtcInitIntersectContext(&context);
            }
        }
        primId = rayHit.hit.primID;
        geomId = rayHit.hit.geomID;
        return hit();
    }

    Spectrum Intersection::Le(const Vec3f &wi) const {
        if (Vec3f::dot(wi, Ns) > 0)
            return primitive->material()->emission.evalAlbedo();
        return {};
    }

    RTCRay Ray::toRTCRay() const {
        RTCRay ray;
        auto _o = o + EPS * d;
        ray.dir_x = d.x();
        ray.dir_y = d.y();
        ray.dir_z = d.z();
        ray.org_x = _o.x();
        ray.org_y = _o.y();
        ray.org_z = _o.z();
        ray.tnear = near;
        ray.tfar = far;
        ray.flags = 0;
        return ray;
    }


}