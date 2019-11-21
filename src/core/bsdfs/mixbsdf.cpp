#include "mixbsdf.h"

namespace miyuki::core {

    Spectrum MixBSDF::evaluate(const ShadingPoint &sp, const Vec3f &wo, const Vec3f &wi) const {
        return mix<Spectrum>(bsdfA->evaluate(sp, wo, wi), bsdfB->evaluate(sp, wo, wi), fraction->evaluate(sp));
    }
    Float MixBSDF::evaluatePdf(const ShadingPoint &sp, const Vec3f &wo, const Vec3f &wi) const {
        return mix<Float>(bsdfB->evaluatePdf(sp, wo, wi), bsdfA->evaluatePdf(sp, wo, wi), fraction->evaluate(sp)[0]);
    }

    void MixBSDF::sample(Point2f u, const ShadingPoint &sp, BSDFSample &sample) const {
        auto frac = fraction->evaluate(sp)[0];
        BSDF *first, *second;
        if (frac < u[0]) {
            u[0] /= frac;
            first = bsdfA.get();
            second = bsdfB.get();
            frac = u[0];
        } else {
            u[0] = (u[0] - frac) / (1.0f - frac);
            first = bsdfB.get();
            second = bsdfA.get();
            frac = 1.0f - u[0];
        }
        first->sample(u, sp, sample);

        // evaluate whole bsdf if not sampled specular
        if ((sample.sampledType & BSDF::ESpecular) == 0) {
            sample.f = mix<Spectrum>(second->evaluate(sp, sample.wo, sample.wi), sample.f, frac);
            sample.pdf = mix<Float>(second->evaluatePdf(sp, sample.wo, sample.wi), sample.pdf, frac);
        }
    }

    BSDF::Type MixBSDF::getBSDFType() const { return BSDF::Type(bsdfA->getBSDFType() | bsdfB->getBSDFType()); }

} // namespace miyuki::core