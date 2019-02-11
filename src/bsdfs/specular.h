//
// Created by Shiina Miyuki on 2019/2/10.
//

#ifndef MIYUKI_SPECULAR_H
#define MIYUKI_SPECULAR_H

#include "bsdf.h"
#include "fresnel.h"

namespace Miyuki {
    class SpecularBSDF : public BSDF {
    public:
        Fresnel *fresnel;

        SpecularBSDF(Fresnel *fresnel, const ColorMap &albedo, const ColorMap &bump = ColorMap())
                : BSDF(BSDFType((int) BSDFType::specular | (int) BSDFType::reflection),
                       albedo, bump),
                  fresnel(fresnel) {}

    protected:
        Spectrum f(const ScatteringEvent &event) const override {
            return {0, 0, 0};
        }

    public:
        Spectrum sample(ScatteringEvent &event) const override {
            event.setWi(Vec3f(-event.wo.x(), event.wo.y(), -event.wo.z()));
            event.pdf = 1;
            return Spectrum(fresnel->eval(cosTheta(event.wi)) * evalAlbedo(event) / absCosTheta(event.wi));
        }

        Float pdf(const Vec3f &wo, const Vec3f &wi) const override {
            return 0.0f;
        }
    };

    class SpecularTransmissionBSDF : public BSDF {
        FresnelDielectric fresnel;
        Float etaA, etaB;
    public:
        SpecularTransmissionBSDF(Float etaA, Float etaB, const ColorMap &albedo, const ColorMap &bump = ColorMap())
                : BSDF(BSDFType((int) BSDFType::specular | (int) BSDFType::transmission),
                       albedo, bump),etaA(etaA),etaB(etaB),
                  fresnel(fresnel) {}

    protected:
        Spectrum f(const ScatteringEvent &event) const override {
            return {0, 0, 0};
        }

    public:
        Spectrum sample(ScatteringEvent &event) const override;

        Float pdf(const Vec3f &wo, const Vec3f &wi) const override {
            return 0.0f;
        }
    };
}

#endif //MIYUKI_SPECULAR_H
