//
// Created by Shiina Miyuki on 2019/3/16.
//

#include "infinite.h"
#include <math/sampling.h>

namespace Miyuki {

    Float InfiniteAreaLight::power() const {
        return envMap.evalAlbedo().max() * worldRadius * worldRadius * PI;
    }

    Spectrum InfiniteAreaLight::L(const Ray &ray) const {
        if (!envMap.image)
            return envMap.evalAlbedo();
        Float sinTheta = ray.d.y();
        Float phi = std::atan2(ray.d.x(), ray.d.z()) + PI;

        Float theta = std::asin(sinTheta) + PI / 2;

        return envMap.evalUV({phi / (2 * PI), theta / PI}) * envMap.evalAlbedo();

    }

    Spectrum InfiniteAreaLight::sampleLi(const Point2f &u, const Intersection &intersection, Vec3f *wi, Float *pdf,
                                         VisibilityTester *tester) const {
        *pdf = 0;
        return {};
    }

    Float InfiniteAreaLight::pdfLi(const Intersection &intersection, const Vec3f &wi) const {
        return 0;
    }

    Spectrum InfiniteAreaLight::sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                                         Float *pdfDir) const {
        return {};
    }

    void InfiniteAreaLight::pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const {
        *pdfPos = *pdfDir = 0;
    }
}