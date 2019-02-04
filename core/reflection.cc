//
// Created by Shiina Miyuki on 2019/2/3.
//

#include "reflection.h"

using namespace Miyuki;

Spectrum
BxDF::sampleF(const Vec3f &wo, Vec3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const {
    *wi = cosineWeightedHemisphereSampling(sample);
    if (wi->y() < 0)wi->y() *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

Float BxDF::Pdf(const Vec3f &wi, const Vec3f &wo) const {
    if (wi.y() * wo.y() <= 0) {
        return 0;
    }
    return fabs(Vec3f::dot(wi, wo)) * INVPI;
}

bool BxDF::matchFlags(BxDFType flags) const {
    return (BxDFType) ((int) flags & (int) type) == type;
}

Spectrum BSDF::f(const Interaction &interaction, BxDFType flags) const {
    Spectrum color(0, 0, 0);
    bool reflect = Vec3f::dot(interaction.wi, interaction.Ng) * Vec3f::dot(interaction.Ng, interaction.wo) > 0;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->matchFlags(flags) &&
            ((reflect && ((int) bxdfs[i]->type & (int) BxDFType::reflection)) ||
             (!reflect && ((int) bxdfs[i]->type & (int) BxDFType::transmission))))
            color += bxdfs[i]->f(interaction.localWo, interaction.localWi);
    return color;
}

void BSDF::add(BxDF *bxDF) {
    assert(nBxDFs < maxBxDFs);
    bxdfs[nBxDFs++] = bxDF;
}

Spectrum ScaledBxDF::f(const Vec3f &wo, const Vec3f &wi) const {
    return Spectrum(scale * bxdf->f(wo, wi));
}

Spectrum
ScaledBxDF::sampleF(const Vec3f &wo, Vec3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const {
    return bxdf->sampleF(wo, wi, sample, pdf, sampledType);
}

Float ScaledBxDF::Pdf(const Vec3f &wi, const Vec3f &wo) const {
    return bxdf->Pdf(wi, wo);
}

Spectrum LambertianReflection::f(const Vec3f &wo, const Vec3f &wi) const {
    return Spectrum(R.color * INVPI);
}
