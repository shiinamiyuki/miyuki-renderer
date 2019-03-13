//
// Created by Shiina Miyuki on 2019/3/7.
//

#ifndef MIYUKI_AREA_H
#define MIYUKI_AREA_H

#include <materials/material.h>
#include "light.h"

namespace Miyuki {
    class AreaLight : public Light {
        const Primitive *primitive;
        Texture ka;
    public:
        AreaLight(const Primitive *primitive) : Light(area), primitive(primitive), ka(primitive->material()->emission) {}

        Float power() const override;

        Spectrum L() const override;

        Spectrum sampleLi(const Point2f &u, const Intersection &intersection, Vec3f *wi, Float *pdf,
                          VisibilityTester *tester) const override;

        Float pdfLi(const Intersection &intersection, const Vec3f &wi) const override;

        Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                          Float *pdfDir) const override;

        void pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const override;

        const Primitive *getPrimitive() const override {
            return primitive;
        }

    };
}
#endif //MIYUKI_AREA_H
