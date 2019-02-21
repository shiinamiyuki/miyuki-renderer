//
// Created by Shiina Miyuki on 2019/2/11.
//

#ifndef MIYUKI_MIXED_H
#define MIYUKI_MIXED_H

#include "bsdf.h"

namespace Miyuki {
    class MixedBSDF : public BSDF {
        std::shared_ptr<BSDF> bsdf1, bsdf2;
        // f = ratio * bsdf1->f() + bsdf2->f()
        Float ratio;
    public:
        MixedBSDF(std::shared_ptr<BSDF> bsdf1, std::shared_ptr<BSDF> bsdf2, Float ratio,const ColorMap&bump);

        /* We will importance sample the bsdfs
         * bsdf1 has discrete probability of ratio/(1 + ratio)
         * bsdf2 has discrete probability if 1/(1 + ratio)
         */
        Spectrum sample(ScatteringEvent &event) const override;

        Float pdf(const Vec3f &wo, const Vec3f &wi, BSDFType flags) const override;

    protected:
        Spectrum f(const ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_MIXED_H
