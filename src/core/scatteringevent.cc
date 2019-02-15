//
// Created by Shiina Miyuki on 2019/2/8.
//

#include "scatteringevent.h"
#include "../bsdfs/bsdf.h"

using namespace Miyuki;


Vec3f ScatteringEvent::worldToLocal(const Vec3f &v) const {
    return Vec3f(Vec3f::dot(localX, v), Vec3f::dot(Ns, v), Vec3f::dot(localZ, v)).normalized();
}

Vec3f ScatteringEvent::localToWorld(const Vec3f &v) const {
    return (v.x() * localX + v.y() * Ns + v.z() * localZ).normalized();
}

void ScatteringEvent::computeLocalCoordinates() {
    const auto &w = info->normal;
    localX = Vec3f::cross((abs(w.x()) > 0.1) ? Vec3f{0, 1, 0} : Vec3f{1, 0, 0}, w);
    localX.normalize();
    localZ = Vec3f::cross(w, localX);
    localZ.normalize();
}

ScatteringEvent::ScatteringEvent(const IntersectionInfo *info, Sampler *sampler)
        : info(info), woW(info->wo), Ns(info->normal), pdf(0), sampler(sampler),flags(BSDFType::all) {
    assert(info && sampler);
    computeLocalCoordinates();
    wo = worldToLocal(woW);
}

const Point2f &ScatteringEvent::uv() const {
    return info->uv;
}

const Vec3f &ScatteringEvent::hitPoint() const {
    return info->hitpoint;
}

const IntersectionInfo *ScatteringEvent::getIntersectionInfo() const {
    return info;
}

Spectrum ScatteringEvent::Le(const Vec3f &wo) const {
    return info->Le(wo);
}

const Vec3f &ScatteringEvent::Ng() const {
    return info->Ng;
}

