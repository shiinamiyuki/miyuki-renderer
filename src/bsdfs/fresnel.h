//
// Created by Shiina Miyuki on 2019/2/10.
//

#ifndef MIYUKI_FRESNEL_H
#define MIYUKI_FRESNEL_H

#include "bsdf.h"

namespace Miyuki {
    inline Vec3f reflect(const Vec3f &wo, const Vec3f &n) {
        return -1 * wo + 2 * Vec3f::dot(wo, n) * n;
    }

    inline bool refract(const Vec3f &wi, const Vec3f &n, Float eta,
                        Vec3f *wt) {
        Float cosThetaI = Vec3f::dot(n, wi);
        Float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
        Float sin2ThetaT = eta * eta * sin2ThetaI;
        if (sin2ThetaT >= 1) return false;

        Float cosThetaT = std::sqrt(1 - sin2ThetaT);

        *wt = -eta * wi + (eta * cosThetaI - cosThetaT) * Vec3f(n);
        return true;
    }

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
