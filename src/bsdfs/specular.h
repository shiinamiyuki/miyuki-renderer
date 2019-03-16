//
// Created by Shiina Miyuki on 2019/3/8.
//

#ifndef MIYUKI_SPECULAR_H
#define MIYUKI_SPECULAR_H

#include "bsdf.h"

namespace Miyuki {
    class SpecularReflection : public BxDF {
        Spectrum R;
    public:
        SpecularReflection(const Spectrum &R) : R(R), BxDF(BSDFLobe::specular | BSDFLobe::reflection) {}

        Spectrum f(const ScatteringEvent &event) const override;

        Float pdf(const ScatteringEvent &event) const override;

        Spectrum sample(ScatteringEvent &event) const override;
    };

    class SpecularTransmission : public BxDF {
        Spectrum R;
        Float etaA, etaB;
        FresnelDielectric dielectric;
    public:
        SpecularTransmission(const Spectrum &R, Float etaA, Float etaB)
                : R(R), BxDF(BSDFLobe::specular | BSDFLobe::reflection), etaA(etaA), etaB(etaB),
                  dielectric(etaA, etaB) {}

        Spectrum f(const ScatteringEvent &event) const override {
            return {};
        }

        Float pdf(const ScatteringEvent &event) const override {
            return 0;
        }

        Spectrum sample(ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_SPECULAR_H
