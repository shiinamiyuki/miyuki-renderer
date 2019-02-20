//
// Created by Shiina Miyuki on 2019/2/11.
//

#include "mixed.h"

using namespace Miyuki;

// TODO: how to handle delta distribution
Spectrum MixedBSDF::f(const ScatteringEvent &event) const {
    auto f1 = bsdf1->f(event);
    auto f2 = bsdf2->f(event);
    return Spectrum(ratio * f1 + f2);

}

Spectrum MixedBSDF::sample(ScatteringEvent &event) const {
    auto p1 = ratio / (ratio + 1);
    Float p = 0;
    Spectrum F;
    if (event.getSampler()->nextFloat() < p1) {
        F = bsdf1->sample(event);
        p = p1;
    } else {
        F = bsdf2->sample(event);
        p = 1 - p1;
    }
    F /= p;
    event.pdf /= p;
    return F;
}

Float MixedBSDF::pdf(const Vec3f &wo, const Vec3f &wi, BSDFType flags) const {
    auto a = bsdf1->matchFlags(flags), b = bsdf2->matchFlags(flags);
    if (a && b) {
        auto pdf1 = bsdf1->pdf(wo, wi, flags);
        auto pdf2 = bsdf2->pdf(wo, wi, flags);
        return (ratio * pdf1 + pdf2) / (ratio + 1);
    } else if (a) {
        return bsdf1->pdf(wo, wi, flags);
    } else if (b) {
        return bsdf2->pdf(wo, wi, flags);
    }
    return 0;
}

MixedBSDF::MixedBSDF(std::shared_ptr<BSDF> bsdf1, std::shared_ptr<BSDF> bsdf2, Float ratio) :
        BSDF(BSDFType((int) bsdf1->getType() | (int) bsdf2->getType()), ColorMap()), bsdf1(bsdf1), bsdf2(bsdf2),
        ratio(ratio) {

}
