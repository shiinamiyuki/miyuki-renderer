//
// Created by Shiina Miyuki on 2019/2/10.
//

#ifndef MIYUKI_FRESNEL_H
#define MIYUKI_FRESNEL_H

#include "bsdf.h"

namespace Miyuki {
    class Fresnel {
    public:
        virtual ~Fresnel() = default;

        virtual Spectrum eval(Float cosI) const = 0;
    };

    class FresnelDielectric : public Fresnel {
        Float etaI, etaT;
    public:
        FresnelDielectric(Float etaI, Float etaT) : etaI(etaI), etaT(etaT) {}

        Spectrum eval(Float cosI) const override;
    };

    class FresnelPerfectSpecular : public Fresnel {
    public:
        FresnelPerfectSpecular() {}

        Spectrum eval(Float cosI) const override { return {1, 1, 1}; }
    };
}
#endif //MIYUKI_FRESNEL_H
