//
// Created by Shiina Miyuki on 2019/3/8.
//

#include "microfacet.h"
#include "core/scatteringevent.h"

namespace Miyuki {

    Spectrum MicrofacetReflection::f(const ScatteringEvent &event) const {
        Float cosThetaO = AbsCosTheta(event.wo), cosThetaI = AbsCosTheta(event.wi);
        Vec3f wh = event.wo + event.wi;
        wh.normalize();
        Spectrum F = fresnel->eval(Vec3f::absDot(wh, event.wi));
        return R * distribution.D(wh) * distribution.G(event.wo, event.wi) * F /
               (4 * cosThetaI * cosThetaO);
    }

    Float MicrofacetReflection::pdf(const ScatteringEvent &event) const {
        if (!SameHemisphere(event.wo, event.wi))
            return 0.0f;
        Vec3f wh = (event.wo + event.wi).normalized();
        // ???
        return distribution.pdf(event.wo, wh) / (4.0f * Vec3f::dot(event.wo, wh));
    }

    Spectrum MicrofacetReflection::sample(ScatteringEvent &event) const {
        auto wh = distribution.sampleWh(event.wo, event.u);
        event.setWi(Reflect(event.wo, wh));
        if (!SameHemisphere(event.wo, event.wi))
            return 0.0f;
        event.pdf = distribution.pdf(event.wo, wh) / (4 * Vec3f::dot(event.wo, wh));
        return f(event);
    }
}