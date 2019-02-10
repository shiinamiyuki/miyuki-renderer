//
// Created by Shiina Miyuki on 2019/2/2.
//

#ifndef MIYUKI_MICROFACET_H
#define MIYUKI_MICROFACET_H

#include "../utils/util.h"
#include "../math/geometry.h"
#include "bsdf.h"

namespace Miyuki {

    class MicrofacetDistribution {
    protected:
        bool sampleVisible;
    public:
        MicrofacetDistribution(bool sampleVis = true) : sampleVisible(sampleVis) {}

        virtual Float D(const Vec3f &wh) = 0;

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
        Float D(const Vec3f &wh) override;

        static Float roughnessToAlpha(Float roughness) {
            roughness = std::max(roughness, (Float) 1e-3);
            Float x = std::log(roughness);
            return 1.62142f + 0.819955f * x + 0.1734f * x * x +
                   0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
        }

        BeckmannDistribution(Float alphaX, Float alphaY, bool sampleVis = true)
                : MicrofacetDistribution(sampleVis), alphaX(alphaX), alphaY(alphaY) {}

        Float lambda(const Vec3f &w) const override;

        Vec3f sampleWh(const Vec3f &wo, const Point2f &u) const override;
    };

    class TrowbridgeReitzDistribution : public MicrofacetDistribution {
    public:

    };

    template<typename T>
    class MicrofacetBSDF : public BSDF {
        static_assert(std::is_base_of<MicrofacetDistribution, T>::value,
                      "T is not derived from MicrofacetDistribution");

    public:


    };
}
#endif //MIYUKI_MICROFACET_H
