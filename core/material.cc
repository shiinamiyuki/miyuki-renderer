//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "material.h"
#include "interaction.h"

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
    auto Ka = kaAt(interaction);
    auto Kd = kdAt(interaction);
    auto Ks = ksAt(interaction);
    auto p1 = Ka.max();
    auto p2 = Kd.max();
    auto p3 = Ks.max();
    auto total = p1 + p2 + p3;
    auto x = sampler.nextFloat() * total;
    BxDFType sampled = BxDFType::none;
    Spectrum color;
    Float pdf = 0;
    Vec3f wi;
    if (total < EPS) {
        *_sampled = sampled;
        *_pdf = 1;
        return {};
    }
    if (x < p1) {
        sampled = BxDFType::emission;
        color = Ka * total / p1;
        pdf = 1;
    } else if (x < p1 + p2) {
        sampled = BxDFType::diffuse;
        color = Kd * INVPI * total / p2;
        auto p = sampler.nextFloat2D();
        wi = cosineWeightedHemisphereSampling(interaction.normal, p.x(), p.y());
        pdf = Vec3f::dot(interaction.normal, wi) / PI;
    } else {
        color = Ks * total / p3;
        Vec3f normal = interaction.normal;
        sampled = static_cast<BxDFType >((int) sampled | (int) BxDFType::specular);
        if (glossiness > 0.01) {
            auto p = sampler.nextFloat2D();
            normal = GGXImportanceSampling(glossiness, normal, p.x(), p.y());
            sampled = static_cast<BxDFType >((int) sampled | (int) BxDFType::glossy);
        }
        if (sampler.nextFloat() < tr) {
            Float n1, n2;
            Float cosI = Vec3f::dot(wo, normal);
            if (cosI < 0) {
                n1 = 1;
                n2 = ior;
                cosI = -cosI;
            } else {
                n1 = ior;
                n2 = 1;
            }
            Float n12 = n1 / n2;
            Float s2 = n12 * n12 * (1 - cosI * cosI);
            Float root = 1 - s2;
            Float T, R;

            if (root >= 0) {
                auto cosT = sqrt(root);
                auto Rpara = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
                auto Rorth = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
                R = (Float) (Rpara * Rpara + Rorth * Rorth) / 2;
            } else {
                R = 1;
            }
            T = 1 - R;
            if (sampler.nextFloat() < R) {
                color /= R;
                wi = reflect(normal, wo);

            } else {
                color /= T;
                sampled = static_cast<BxDFType >((int) sampled | (int) BxDFType::refraction);
                wi = n12 * wo + (n12 * cosI - sqrt(root)) * normal;
            }
        } else {
            color /= 1 - tr;
            wi = reflect(normal, wo);
        }
        pdf = 1;
    }
    if (_sampled) {
        *_sampled = sampled;
    }
    *_wi = wi;
    *_pdf = pdf;
    return color;
}

Float Material::f(BxDFType type, const Interaction &interaction, const Vec3f &wo, const Vec3f &wi) const {
    if (type == BxDFType::diffuse) {
        return INVPI;
    } else if (type == BxDFType::specular) {
        return 0;
    } else if (hasBxDFType(type, BxDFType::glossy)) {
        if (!hasBxDFType(type, BxDFType::refraction)) {
            Vec3f microfacet = (wi - wo).normalized();
            Float distribution = GGXDistribution(interaction.normal, microfacet, glossiness);
            return distribution;
        } else {
            return 0; // TODO: glossy refraction
        }
    }
    return 0;
}

Spectrum Material::kaAt(const Interaction &i) const {
    if (!kaMap) {
        return ka;
    }
    auto color = kaMap->sample(textCoord(i));
    color *= ka;
    return color;
}

Spectrum Material::kdAt(const Interaction &i) const {
    if (!kdMap) {
        return kd;
    }
    auto color = kdMap->sample(textCoord(i));;
    color *= kd;
    return color;
}

Spectrum Material::ksAt(const Interaction &i) const {
    if (!ksMap) {
        return ks;
    }
    auto color = ksMap->sample(textCoord(i));
    color *= ks;
    return color;
}

const Point2f Material::textCoord(const Interaction &interaction) const {
    const auto &primitive = *interaction.primitive;
    const auto &uv = interaction.uv;
    return pointOnTriangle(primitive.textCoord[0], primitive.textCoord[1], primitive.textCoord[2], uv.x(), uv.y());
}

