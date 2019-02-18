//
// Created by Shiina Miyuki on 2019/2/17.
//

#include "infinite.h"

using namespace Miyuki;

Spectrum InfiniteLight::sampleLi(const Point2f &u, const IntersectionInfo &info, Vec3f *wi, Float *pdf,
                                 VisibilityTester *tester) const {
    return Miyuki::Spectrum();
}

Spectrum InfiniteLight::sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                                 Float *pdfDir) const {
    return Miyuki::Spectrum();
}

Float InfiniteLight::pdfLi(const IntersectionInfo &info, const Vec3f &wi) const {
    return 0;
}

void InfiniteLight::pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const {

}

InfiniteLight::InfiniteLight(ColorMap &albedo) : albedo(albedo) {
    type = Light::Type::infinite;
    ka = albedo.color;
}
