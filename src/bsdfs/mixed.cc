//
// Created by Shiina Miyuki on 2019/2/11.
//

#include "mixed.h"

using namespace Miyuki;

Spectrum MixedBSDF::f(const ScatteringEvent &event) const {
    auto f1 = bsdf1->f(event);
    auto f2 = bsdf2->f(event);
    return Spectrum((ratio * f1 + f2) / (ratio + 1));

}

Spectrum MixedBSDF::sample(ScatteringEvent &event) const {
    auto p1 = ratio / (ratio + 1);
    Float p = 0;
    Spectrum F;
    if (event.getSampler()->nextFloat() < p1) {
        F = bsdf1->sample(event);
        event.pdf = (ratio * event.pdf + bsdf2->pdf(event.wo, event.wi)) / (ratio + 1);
        p = p1;
    } else {
        F = bsdf2->sample(event);
        event.pdf = (ratio * bsdf1->pdf(event.wo, event.wi) + event.pdf) / (ratio + 1);
        p = 1 - p1;
    }
    F /= p;
    return F;
}

Float MixedBSDF::pdf(const Vec3f &wo, const Vec3f &wi) const {
    auto pdf1 = bsdf1->pdf(wo, wi);
    auto pdf2 = bsdf2->pdf(wo, wi);
    return (ratio * pdf1 + pdf2) / (ratio + 1);
}

MixedBSDF::MixedBSDF(BSDF *bsdf1, BSDF *bsdf2, Float ratio) :
        BSDF(BSDFType((int) bsdf1->getType() | (int) bsdf2->getType()), ColorMap()), bsdf1(bsdf1), bsdf2(bsdf2),
        ratio(ratio) {

}
