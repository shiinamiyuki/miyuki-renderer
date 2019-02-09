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
    public:
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
    public:
        Float D(const Vec3f &wh) override;
    };
}
#endif //MIYUKI_MICROFACET_H
