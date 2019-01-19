//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "material.h"

using namespace Miyuki;

Spectrum Material::sampleF(Sampler &sampler,
                           const Interaction &interaction,
                           const Vec3f &wo,
                           Vec3f *_wi,
                           Float *_pdf,
                           BxDFType sampleType,
                           BxDFType *_sampled) const {
    // TODO: now we only handle `sampleType` = BxDFType::all, fix this
    // TODO: consider pre-compute max reflectivity
    auto p1 = ka.max();
    auto p2 = kd.max();
    auto p3 = ks.max();
    auto total = p1 + p2 + p3;
    auto x = sampler.nextFloat() * total;
    BxDFType sampled = BxDFType::none;
    Spectrum color;
    Float pdf = 0;
    Float x1 = sampler.nextFloat(), x2 = sampler.nextFloat(); // used to determine wi
    Vec3f wi;
    if (total < EPS) {
        *_sampled = sampled;
        *_pdf = 1;
        return {};
    }
    if (x < p1) {
        sampled = BxDFType::emission;
        color = ka * total / p1;
        pdf = 1;
    } else if (x < p1 + p2) {
        sampled = BxDFType::diffuse;
        color = kd * INVPI * total / p2;
        wi = cosineWeightedHemisphereSampling(interaction.norm, x1, x2);
        pdf = Vec3f::dot(interaction.norm, wi) / PI;
    } else {
        sampled = BxDFType::specular;
        color = ks * total / p3;
        pdf = 1;
        wi = reflect(interaction.norm, wo);
    }
    if (_sampled) {
        *_sampled = sampled;
    }
    *_wi = wi;
    *_pdf = pdf;
    return color;
}
