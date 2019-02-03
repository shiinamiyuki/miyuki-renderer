//
// Created by Shiina Miyuki on 2019/2/3.
//

#include "reflection.h"
using namespace Miyuki;
Spectrum
BxDF::sampleF(const Vec3f &wo, Vec3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const {
    *wi = cosineWeightedHemisphereSampling(sample);
    if (wi->z() < 0)wi->z() *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

Float BxDF::Pdf(const Vec3f &wi, const Vec3f &wo) const {
    if (wi.z() * wo.z() <= 0) {
        return 0;
    }
    return fabs(Vec3f::dot(wi, wo)) * INVPI;
}

bool BxDF::matchFlags(BxDFType) const {
    return false;
}
