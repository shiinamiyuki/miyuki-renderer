//
// Created by Shiina Miyuki on 2019/2/2.
//

#ifndef MIYUKI_MICROFACET_H
#define MIYUKI_MICROFACET_H

#include "../utils/util.h"
#include "../math/geometry.h"
#include "bsdf.h"
#include "fresnel.h"

namespace Miyuki {

    class MicrofacetDistribution {
    protected:
        bool sampleVisible;
    public:
        MicrofacetDistribution(bool sampleVis = true) : sampleVisible(sampleVis) {}

        virtual Float D(const Vec3f &wh) const = 0;

        virtual Float lambda(const Vec3f &w) const = 0;

        Float G1(const Vec3f &w) const {
            return 1 / (1 + lambda(w));
        }

        Float G(const Vec3f &wo, const Vec3f &wi) const {
            return 1 / (1 + lambda(wo) + lambda(wi));
        }

        virtual Vec3f sampleWh(const Vec3f &wo,
                               const Point2f &u) const = 0;

        Float Pdf(const Vec3f &wo, const Vec3f &wh) const;

        virtual ~MicrofacetDistribution() = default;
    };

    class BeckmannDistribution : public MicrofacetDistribution {
        const Float alphaX, alphaY;
    public:
        Float D(const Vec3f &wh) const override;

        inline static Float roughnessToAlpha(Float roughness) {
//            roughness = std::max(roughness, (Float) 1e-3);
//            Float x = std::log(roughness);
//            return std::max(1.62142f + 0.819955f * x + 0.1734f * x * x +
//                   0.0171201f * x * x * x + 0.000640711f * x * x * x * x, 1e-5f);
            return std::max(roughness *roughness,1e-5f);
        }

        BeckmannDistribution(Float alphaX, Float alphaY, bool sampleVis = true)
                : MicrofacetDistribution(sampleVis), alphaX(alphaX), alphaY(alphaY) {}

        Float lambda(const Vec3f &w) const override;

        Vec3f sampleWh(const Vec3f &wo, const Point2f &u) const override;
    };

    class TrowbridgeReitzDistribution : public MicrofacetDistribution {
        const Float alphaX, alphaY;
    public:
        inline static Float roughnessToAlpha(Float roughness) {
//            roughness = std::max(roughness, (Float) 1e-3);
//            Float x = std::log(roughness);
//            return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
//                   0.000640711f * x * x * x * x;
            return std::max(roughness *roughness,1e-5f);
        }

        TrowbridgeReitzDistribution(Float alphaX, Float alphaY, bool sampleVis = true)
                : MicrofacetDistribution(sampleVis), alphaX(alphaX), alphaY(alphaY) {}

        Float D(const Vec3f &wh) const override;

        Float lambda(const Vec3f &w) const override;

        Vec3f sampleWh(const Vec3f &wo, const Point2f &u) const override;
    };


    template<typename T>
    class MicrofacetBSDF : public BSDF {
        static_assert(std::is_base_of<MicrofacetDistribution, T>::value,
                      "T is not derived from MicrofacetDistribution");
        T distribution;
        Fresnel *fresnel;
    public:
        MicrofacetBSDF(const T &d, Fresnel *f, const ColorMap &albedo, const ColorMap &bump = ColorMap())
                : BSDF(BSDFType((int) BSDFType::glossy | (int) BSDFType::reflection),
                       albedo, bump), distribution(d), fresnel(f) {}

    protected:
        Spectrum f(const ScatteringEvent &event) const override {
            const auto &wo = event.wo, wi = event.wi;
            auto wh = wi + wo;
            Float cosThetaO = absCosTheta(wo), cosThetaI = absCosTheta(wi);
            if (cosThetaI == 0 || cosThetaO == 0) return {};
            if (wh.x() == 0 && wh.y() == 0 && wh.z() == 0) return {};
            wh.normalize();
            auto F = fresnel->eval(Vec3f::dot(wi, wh));
            return Spectrum(evalAlbedo(event) * distribution.D(wh) * distribution.G(wo, wi) * F /
                            (4 * cosThetaI * cosThetaO));
        }

    public:
        Spectrum sample(ScatteringEvent &event) const override {
            const auto &wo = event.wo;
            if (wo.y() == 0) return 0.;
            Vec3f wh = distribution.sampleWh(wo, event.getSampler()->nextFloat2D());
            event.setWi(reflect(wo, wh).normalized());
            if (!sameHemisphere(wo, event.wi)) return {};

            // Compute PDF of _wi_ for microfacet reflection
            event.pdf = distribution.Pdf(wo, wh) / (4 * Vec3f::dot(wo, wh));
            return eval(event);
        }

        Float pdf(const Vec3f &wo, const Vec3f &wi, BSDFType) const override {
            if (!sameHemisphere(wo, wi)) return 0;
            Vec3f wh = (wo + wi).normalized();
            return distribution.Pdf(wo, wh) / (4 * Vec3f::dot(wo, wh));
        }

    };
}
#endif //MIYUKI_MICROFACET_H
