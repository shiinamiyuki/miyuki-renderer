//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "interaction.h"

using namespace Miyuki;


Vec3f Interaction::worldToLocal(const Vec3f &v) const {
    return Vec3f(Vec3f::dot(localX, v), Vec3f::dot(normal, v), Vec3f::dot(localZ, v));
}

Vec3f Interaction::localToWorld(const Vec3f &v) const {
    return v.x() * localX + v.y() * normal + v.z() * localZ;
}

void Interaction::computeLocalCoordinate() {
    const auto &w = normal;
    localX = Vec3f::cross((abs(w.x()) > 0.1) ? Vec3f{0, 1, 0} : Vec3f{1, 0, 0}, w);
    localX.normalize();
    localZ = Vec3f::cross(w, localX);
    localZ.normalize();
}

Point2f Interaction::textureCoord() const {
    return pointOnTriangle(primitive->textCoord[0], primitive->textCoord[1], primitive->textCoord[2], uv.x(), uv.y());
}

Ray Interaction::spawnWi() const {
    return {hitpoint, localToWorld(localWi)};
}
