//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "light.h"

using namespace Miyuki;

bool VisibilityTester::visible(const Scene &scene) const {
    Intersection intersection(shadowRay);
    intersection.intersect(scene);
    return intersection.hit() &&
           intersection.geomID() == targetGeomID
           && intersection.primID() == targetPrimID;
}

AreaLight::AreaLight(const Mesh::MeshInstance::Primitive &_primitive, const Spectrum &ka)
        : Light(ka), primitive(&_primitive) {
    type = Type::area;
    area = Vec3f::cross(primitive->vertices[1] - primitive->vertices[0],
                        primitive->vertices[2] - primitive->vertices[0]).length() / 2;
}

Spectrum
AreaLight::sampleLi(const Point2f &u, const Interaction &interaction, Vec3f *wi, Float *pdf,
                    VisibilityTester *tester) const {
    Float x = u.x(), y = u.y();
    if (x + y > 1) {
        x = 1 - x;
        y = 1 - y;
    }
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

Spectrum
AreaLight::sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos, Float *pdfDir) const {
    Float x = u1.x(), y = u1.y();
    if (x + y > 1) {
        x = 1 - x;
        y = 1 - y;
    }
    auto p = pointOnTriangle(primitive->vertices[0],
                             primitive->vertices[1],
                             primitive->vertices[2],
                             x,
                             y);
    *normal = primitive->normalAt(u1);
    *pdfPos = 1 / area;
    Vec3f dir = cosineWeightedHemisphereSampling(*normal, u2.x(), u2.y());
    *ray = Ray(p, dir);
    *pdfDir = Vec3f::dot(dir, *normal) * INVPI;
    return ka;
}

Float AreaLight::power() const{
    return ka.max() * area;
}

Spectrum PointLight::sampleLi(const Point2f &u, const Interaction &interaction, Vec3f *wi, Float *pdf,
                              VisibilityTester *tester) const {

    auto Wi = (interaction.hitpoint - position);
    auto dist = Wi.lengthSquared();
    Wi.normalize();
    *pdf = dist;
    *wi = Wi;
    tester->shadowRay = Ray(position, *wi);
    tester->targetPrimID = interaction.primID;
    tester->targetGeomID = interaction.geomID;
    return ka;
}

Spectrum PointLight::sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                              Float *pdfDir) const {
    return {};
}
