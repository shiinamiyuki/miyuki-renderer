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

        Spectrum f(const ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_LAMBERTIAN_H
