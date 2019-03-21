//
// Created by Shiina Miyuki on 2019/3/5.
//

#include "bsdf.h"
#include "math/sampling.h"
#include "core/scatteringevent.h"

namespace Miyuki {

    Float BxDF::pdf(const ScatteringEvent &event) const {
        return SameHemisphere(event.wo, event.wi) ? AbsCosTheta(event.wi) * INVPI : 0;
    }

    Spectrum BxDF::sample(ScatteringEvent &event) const {
        auto wi = CosineWeightedHemisphereSampling(event.u);
        if (event.wo.y() < 0) {
            wi.y() *= -1;
        }
        event.setWi(wi);
        event.bsdfLobe = lobe;
        event.pdf = pdf(event);
        return f(event);
    }

    Spectrum BSDF::f(const ScatteringEvent &event, BSDFLobe lobe) const {
        Spectrum bsdf;
        if (event.wo.y() == 0)return {};
        bool reflect = SameHemisphere(event.wo, event.wi);
        for (int i = 0; i < nBxDFs; i++) {
            if (bxdfs[i]->matchFlag(lobe) &&
                ((reflect && bxdfs[i]->matchFlag(BSDFLobe::reflection))
                 || (!reflect && bxdfs[i]->matchFlag(BSDFLobe::transmission)))) {
                bsdf += bxdfs[i]->f(event);
            }
        }
        return bsdf;
    }

    Float BSDF::pdf(const ScatteringEvent &event, BSDFLobe lobe) const {
        Float p = 0;
        int cnt = 0;
        for (int i = 0; i < nBxDFs; i++) {
            if (bxdfs[i]->matchFlag(lobe)) {
                p += bxdfs[i]->pdf(event);
                cnt++;
            }
        }
        return cnt != 0 ? p / cnt : 0.0f;
    }

    Spectrum BSDF::sample(ScatteringEvent &event, BSDFLobe lobe) const {
        int matched = numComponents(lobe);
        if (matched == 0) {
            event.bsdfLobe = 0;
            event.pdf = 0;
            return {};
        }
        int comp = std::min<int>(matched - 1, std::floor(event.u[0] * matched));
        // remap event.u
        event.u[0] = std::min(event.u[0] * matched - comp, 1.0f - 1e-6f);
        BxDF *bxdf = nullptr;
        int cnt = 0;
        for (int i = 0; i < nBxDFs; i++) {
            if (bxdfs[i]->matchFlag(lobe)) {
                if (cnt == comp) {
                    bxdf = bxdfs[i];
                    break;
                }
                cnt++;
            }
        }
        Spectrum bsdf = bxdf->sample(event);
        if (matched > 1 && !(bxdf->matchFlag(BSDFLobe::specular))) {
            for (int i = 0; i < nBxDFs; i++) {
                if (bxdf != bxdfs[i] && bxdfs[i]->matchFlag(lobe)) {
                    event.pdf += bxdfs[i]->pdf(event);
                }
            }
        }
        if (matched > 1) {
            event.pdf /= matched;
        }

        CHECK(event.pdf >= 0);

        if (event.pdf == 0) {
            event.bsdfLobe = 0;
            return {};
        }

        if (!bxdf->matchFlag(BSDFLobe::specular)) {
            bool reflect = Vec3f::dot(event.woW, event.Ng()) * Vec3f::dot(event.wiW, event.Ng()) > 0;
            bsdf = Spectrum{};
            for (int i = 0; i < nBxDFs; i++) {
                if (bxdfs[i]->matchFlag(lobe) &&
                    ((reflect && bxdfs[i]->matchFlag(BSDFLobe::reflection))
                     || (!reflect && bxdfs[i]->matchFlag(BSDFLobe::transmission)))) {
                    bsdf += bxdfs[i]->f(event);
                }
            }
        }
        return bsdf;
    }
}