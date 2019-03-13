//
// Created by Shiina Miyuki on 2019/3/6.
//

#ifndef MIYUKI_LIGHT_H
#define MIYUKI_LIGHT_H

#include "core/geometry.h"
#include "core/ray.h"
#include "core/spectrum.h"

namespace Miyuki {
    class Scene;

    struct VisibilityTester {
        Ray shadowRay;
        int32_t geomId, primId;

        VisibilityTester() : geomId(-1), primId(-1) {}

        bool visible(Scene &scene);
    };

    class Light {
    public:
        static const int deltaPosition = 1;
        static const int deltaDirection = 2;
        static const int area = 4;
        static const int infinite = 8;
        int type;

        virtual Float power() const = 0;

        virtual Spectrum L() const = 0;

        Light(int type) : type(type) {}

        virtual ~Light() {}

        virtual Spectrum
        sampleLi(const Point2f &u, const Intersection &, Vec3f *wi, Float *pdf, VisibilityTester *) const = 0;

        virtual Float pdfLi(const Intersection &, const Vec3f &wi) const = 0;

        virtual Spectrum sampleLe(const Point2f &u1,
                                  const Point2f &u2,
                                  Ray *ray,
                                  Vec3f *normal,
                                  Float *pdfPos,
                                  Float *pdfDir) const = 0;

        virtual void pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const = 0;

        virtual const Primitive *getPrimitive() const { return nullptr; }
    };
}
#endif //MIYUKI_LIGHT_H
