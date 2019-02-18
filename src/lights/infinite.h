//
// Created by Shiina Miyuki on 2019/2/17.
//

#ifndef MIYUKI_INFINITE_H
#define MIYUKI_INFINITE_H

#include "light.h"
namespace Miyuki{
    class InfiniteLight : public Light{
        ColorMap albedo;
    public:
        InfiniteLight(ColorMap & albedo);
        Spectrum sampleLi(const Point2f &u, const IntersectionInfo &info, Vec3f *wi, Float *pdf,
                          VisibilityTester *tester) const override;

        Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                          Float *pdfDir) const override;

        Float pdfLi(const IntersectionInfo &info, const Vec3f &wi) const override;

        void pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const override;
    };
}
#endif //MIYUKI_INFINITE_H
