//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "ray.h"
using namespace Miyuki;
RTCRay Ray::toRTCRay() const {
    RTCRay ray;
    ray.dir_x = d.x();
    ray.dir_y = d.y();
    ray.dir_z = d.z();
    ray.org_x = o.x();
    ray.org_y = o.y();
    ray.org_z = o.z();
    ray.tnear = EPS;
    ray.tfar = INF;
    ray.flags = 0;
    return ray;
}

Ray::Ray(const RTCRay &ray) {

}
