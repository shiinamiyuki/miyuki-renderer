//
// Created by Shiina Miyuki on 2019/3/6.
//

#ifndef MIYUKI_LAMBERTIAN_H
#define MIYUKI_LAMBERTIAN_H

#include "bsdf.h"

namespace Miyuki {
    class LambertianReflection : public BxDF {
        Spectrum R;
    public:
        LambertianReflection(const Spectrum &R) : R(R), BxDF(BSDFLobe::diffuse | BSDFLobe::reflection) {}

        Point2f invert(const Vec3f &wo, const Vec3f &wi) const override;

        Spectrum f(const ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_LAMBERTIAN_H
