//
// Created by Shiina Miyuki on 2019/2/2.
//

#include "microfacet.h"

using namespace Miyuki;

Miyuki::Float Miyuki::BeckmannDistribution::D(const Miyuki::Vec3f &wh) {
    Float t2 = tan2Theta(wh);
    if (std::isinf(t2)) return 0.0;
    Float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
    return std::exp(-t2 * (cos2Phi(wh) / (alphaX * alphaX) +
                           sin2Phi(wh) / (alphaY * alphaY))) /
           (PI * alphaX * alphaY * cos4Theta);
}

Float BeckmannDistribution::lambda(const Vec3f &w) const {
    return 0;
}

Vec3f BeckmannDistribution::sampleWh(const Vec3f &wo, const Point2f &u) const {
    return Vec3f();
}
