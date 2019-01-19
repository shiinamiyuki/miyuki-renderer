//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "light.h"

using namespace Miyuki;

bool VisibilityTester::visible(RTCScene scene) const {
    Intersection intersection(shadowRay);
    intersection.intersect(scene);
    return intersection.hit() &&
           intersection.geomID() == targetGeomID
           && intersection.primID() == targetPrimID;
}

AreaLight::AreaLight(const Mesh::MeshInstance::Primitive &_primitive, const Spectrum &ka)
        : Light(ka), primitive(&_primitive) {
    area = Vec3f::cross(primitive->vertices[1] - primitive->vertices[0],
                        primitive->vertices[2] - primitive->vertices[0]).length() / 2;
}

Spectrum
AreaLight::sampleLi(const Point2f &u, const Interaction &interaction, Vec3f *wi, Float *pdf,
                    VisibilityTester *tester) const {
    Float x = u.x(), y = u.y();
    auto p = pointOnTriangle(primitive->vertices[0],
                             primitive->vertices[1],
                             primitive->vertices[2],
                             x,
                             y);
    auto Wi = (interaction.hitpoint - p);
    auto dist = Wi.lengthSquared();
    *wi = Wi / sqrt(dist);
    *pdf = dist / (Vec3f::dot(primitive->normalAt(u), *wi) * area);
    tester->shadowRay = Ray(p, *wi);
    tester->targetPrimID = interaction.primID;
    tester->targetGeomID = interaction.geomID;
    return ka;
}
