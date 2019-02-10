//
// Created by Shiina Miyuki on 2019/2/3.
//

#include "bsdf.h"
#include "../core/scatteringevent.h"

using namespace Miyuki;

Spectrum BSDF::sample(ScatteringEvent &event) const {
    event.setWi(cosineWeightedHemisphereSampling(event.getSampler()->nextFloat2D()));
    event.pdf = pdf(event.wo, event.wi);
    event.sampledType = type;
    return eval(event);
}

inline bool sameHemisphere(const Vec3f &wo, const Vec3f &wi) {
    return wo.y() * wi.y() >= 0;
}

Float BSDF::pdf(const Vec3f &wo, const Vec3f &wi) const {
    return sameHemisphere(wo, wi) ? absCosTheta(wi) * INVPI : 0;
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

