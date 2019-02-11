//
// Created by Shiina Miyuki on 2019/2/3.
//

#include "bsdf.h"
#include "../core/scatteringevent.h"

using namespace Miyuki;

Spectrum BSDF::sample(ScatteringEvent &event) const {
    event.setWi(cosineWeightedHemisphereSampling(event.getSampler()->nextFloat2D()));
    event.sampledType = type;
    event.pdf = pdf(event.wo, event.wi, event.sampledType);
    return eval(event);
}


Float BSDF::pdf(const Vec3f &wo, const Vec3f &wi, BSDFType flags) const {
    if (matchFlags(flags))
        return sameHemisphere(wo, wi) ? absCosTheta(wi) * INVPI : 0;
    else
        return 0;
}

Spectrum BSDF::evalAlbedo(const ScatteringEvent &event) const {
    return albedo.sample(event.uv());
}

Spectrum BSDF::eval(const ScatteringEvent &event) const {
    bool reflect = Vec3f::dot(event.wiW, event.getIntersectionInfo()->Ng) *
                   Vec3f::dot(event.woW, event.getIntersectionInfo()->Ng) > 0;
    if (((reflect && ((int) type & (int) BSDFType::reflection)) ||
         (!reflect && ((int) type & (int) BSDFType::transmission))))
        return f(event);
    return {};
}

