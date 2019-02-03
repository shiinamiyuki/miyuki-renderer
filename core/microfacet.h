//
// Created by Shiina Miyuki on 2019/2/2.
//

#ifndef MIYUKI_MICROFACET_H
#define MIYUKI_MICROFACET_H

#include "util.h"
#include "geometry.h"

namespace Miyuki {
    inline Float cosTheta(const Vec3f &wh) {
        return wh.x() + wh.y() + wh.z();
    }

    inline Float cos2Theta(const Vec3f &wh) {
        auto c = cosTheta(wh);
        return c * c;
    }

    inline Float tan2Theta(const Vec3f &wh) {
        /* sin^2 + cos^2 = 1
         * tan^2 + 1 = 1/cos^2
         * */
        return 1 / cos2Theta(wh) - 1;
    }

    class MicrofacetDistribution {
    public:
        virtual Float D(const Vec3f &wh) = 0;

        virtual ~MicrofacetDistribution() = default;
    };

    class BeckmannDistribution : public MicrofacetDistribution {
    public:
        Float D(const Vec3f &wh) override;
    };
}
#endif //MIYUKI_MICROFACET_H
