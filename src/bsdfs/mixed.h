//
// Created by Shiina Miyuki on 2019/2/11.
//

#ifndef MIYUKI_MIXED_H
#define MIYUKI_MIXED_H

#include "bsdf.h"

namespace Miyuki {
    class MixedBSDF : public BSDF {
        BSDF *bsdf1, *bsdf2;
        Float ratio; // f(bsdf1) / f(bsdf2)
    public:
        MixedBSDF(BSDF *bsdf1, BSDF *bsdf2, Float ratio = 1);

        Spectrum sample(ScatteringEvent &event) const override;

        Float pdf(const Vec3f &wo, const Vec3f &wi) const override;

    protected:
        Spectrum f(const ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_MIXED_H
