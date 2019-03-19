//
// Created by Shiina Miyuki on 2019/3/16.
//

#ifndef MIYUKI_INFINITE_H
#define MIYUKI_INFINITE_H

#include "light.h"
#include <core/texture.h>

namespace Miyuki {
    class InfiniteAreaLight : public Light {
        Texture envMap;
        Float worldRadius;
    public:
        InfiniteAreaLight(Float worldRadius, Texture envMap)
                : worldRadius(worldRadius), envMap(std::move(envMap)), Light(area | infinite) {}

        Float power() const override;

        Spectrum L(const Ray& ray) const override;

        Spectrum sampleLi(const Point2f &u, const Intersection &intersection, Vec3f *wi, Float *pdf,
                          VisibilityTester *tester) const override;

        Float pdfLi(const Intersection &intersection, const Vec3f &wi) const override;

        Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                          Float *pdfDir) const override;

        void pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const override;

    };
}
#endif //MIYUKI_INFINITE_H
