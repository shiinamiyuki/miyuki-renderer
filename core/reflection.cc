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

Float BxDF::Pdf(const Vec3f &wo, const Vec3f &wi) const {
    if (wi.y() * wo.y() <= 0) {
        return 0;
    }
    return fabs(wi.x() + wi.y() + wi.z()) * INVPI;
}

bool BxDF::matchFlags(BxDFType flags) const {
    return (BxDFType) ((int) flags & (int) type) == type;
}

Spectrum BSDF::f(const Vec3f &woW, const Vec3f &wiW, BxDFType flags) const {
    Spectrum color(0, 0, 0);
    auto wi = worldToLocal(wiW);
    auto wo = worldToLocal(woW);
    bool reflect = Vec3f::dot(wiW, Ng) * Vec3f::dot(woW, Ng) > 0;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->matchFlags(flags) &&
            ((reflect && ((int) bxdfs[i]->type & (int) BxDFType::reflection)) ||
             (!reflect && ((int) bxdfs[i]->type & (int) BxDFType::transmission))))
            color += bxdfs[i]->f(wo, wi);
    return color;
}

void BSDF::add(BxDF *bxDF) {
    assert(nBxDFs < maxBxDFs);
    bxdfs[nBxDFs++] = bxDF;
}

BSDF::BSDF(const Interaction &interaction) : eta(1.0), Ns(interaction.normal) {
    computeLocalCoordinates();
}

Spectrum
BSDF::sampleF(const Vec3f &woW, Vec3f *wiW, const Point2f &u, Float *pdf, BxDFType type, BxDFType *sampledType) const {
    auto matchComp = numComponents(type);
    if (!matchComp) {
        *pdf = 0;
        return {};
    }
    int32_t comp = std::min<int32_t>(std::floor(u[0] * matchComp), matchComp - 1);
    auto cnt = 0;
    BxDF *bxdf = nullptr;
    for (int i = 0; i < nBxDFs; i++) {
        if (bxdfs[i]->matchFlags(type) && cnt == comp) {
            bxdf = bxdfs[i];
            break;
        }
        cnt++;
    }
    Point2f uRemapped(u[0] * matchComp - comp, u[1]);
    if (sampledType)*sampledType = bxdf->type;
    Vec3f wo = worldToLocal(woW);
    Vec3f wi;
    auto F = bxdf->sampleF(wo, &wi, uRemapped, pdf, sampledType);
    if (*pdf == 0)
        return {};
    if (!((int) bxdf->type & (int) BxDFType::specular) && matchComp > 1) {
        for (int i = 0; i < nBxDFs; i++) {
            if (bxdfs[i]->matchFlags(type) && bxdfs[i] != bxdf) {
                *pdf += bxdfs[i]->Pdf(wo, wi);
            }
        }
    }
    if (matchComp > 1) {
        *pdf /= matchComp;
    }
    if (!((int) bxdf->type & (int) BxDFType::specular) && matchComp > 1) {
        bool reflect = Vec3f::dot(*wiW, Ng) * Vec3f::dot(woW, Ng) > 0;
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i]->matchFlags(type) &&
                ((reflect && ((int) bxdfs[i]->type & (int) BxDFType::reflection)) ||
                 (!reflect && ((int) bxdfs[i]->type & (int) BxDFType::transmission))))
                F += bxdfs[i]->f(wo, wi);
    }
    return F;
}

int32_t BSDF::numComponents(BxDFType flags) const {
    int32_t num = 0;
    for (int i = 0; i < nBxDFs; i++)
        if (bxdfs[i]->matchFlags(flags))
            num++;
    return num;
}

Vec3f BSDF::worldToLocal(const Vec3f &v) const {
    return Vec3f(Vec3f::dot(localX, v), Vec3f::dot(Ns, v), Vec3f::dot(localZ, v));
}

Vec3f BSDF::localToWorld(const Vec3f &v) const {
    return v.x() * localX + v.y() * Ns + v.z() * localZ;
}

void BSDF::computeLocalCoordinates() {
    const auto &w = Ns;
    localX = Vec3f::cross((abs(w.x()) > 0.1) ? Vec3f{0, 1, 0} : Vec3f{1, 0, 0}, w);
    localX.normalize();
    localZ = Vec3f::cross(w, localX);
    localZ.normalize();
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
    return Spectrum(R * INVPI);
}
