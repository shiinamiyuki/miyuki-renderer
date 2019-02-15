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

AreaLight::AreaLight(const Primitive &_primitive, const Spectrum &ka)
        : Light(ka), primitive(&_primitive) {
    type = Type::area;
    area = _primitive.area;
}

Spectrum
AreaLight::sampleLi(const Point2f &u, const IntersectionInfo &info, Vec3f *wi, Float *pdf,
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
    auto Wi = (p - info.hitpoint);
    auto dist = Wi.lengthSquared();
    *wi = Wi / sqrt(dist);
    *pdf = dist / (Vec3f::dot(primitive->normalAt(u), -1 * *wi) * area);
    tester->shadowRay = Ray(p, *wi * -1);
    tester->targetPrimID = info.primID;
    tester->targetGeomID = info.geomID;
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

Float AreaLight::power() const {
    return ka.max() * area;
}

Float AreaLight::pdfLi(const IntersectionInfo &info, const Vec3f &wi) const {
    return primitive->pdf(info, wi);
}

void AreaLight::pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const {
    *pdfPos = 1 / area;
    *pdfDir = Vec3f::dot(ray.d, normal) * INVPI;
}

Spectrum PointLight::sampleLi(const Point2f &u, const IntersectionInfo &info, Vec3f *wi, Float *pdf,
                              VisibilityTester *tester) const {

    auto Wi = (position - info.hitpoint);
    auto dist = Wi.lengthSquared();
    Wi.normalize();
    *pdf = dist;
    *wi = Wi;
    tester->shadowRay = Ray(position, *wi * -1);
    tester->targetPrimID = info.primID;
    tester->targetGeomID = info.geomID;
    return ka;
}

Spectrum PointLight::sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                              Float *pdfDir) const {
    return {};
}

Float PointLight::pdfLi(const IntersectionInfo &info, const Vec3f &wi) const {
    return 0;
}

void PointLight::pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const {
    *pdfPos = 0;
}
