//
// Created by Shiina Miyuki on 2019/2/3.
//

#include "bsdf.h"
#include "../core/scatteringevent.h"
#include "../core/mesh.h"

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
    auto &primitive = *event.getIntersectionInfo()->primitive;
    auto uv = pointOnTriangle(primitive.textCoord[0], primitive.textCoord[1], primitive.textCoord[2],
                              event.uv().x(), event.uv().y());
    return albedo.sample(uv);
}

// TODO: Ng and Ns
Spectrum BSDF::eval(const ScatteringEvent &event) const {
    bool reflect = Vec3f::dot(event.wiW, event.Ns) *
                   Vec3f::dot(event.woW, event.Ns) > 0;
    if (((reflect && ((int) type & (int) BSDFType::reflection)) ||
         (!reflect && ((int) type & (int) BSDFType::transmission))))
        return f(event);
    return {};
}

bool BSDF::hasBump() const {
    return bump.hasMapping();
}

Vec3f BSDF::evalBump(const ScatteringEvent &event) const {
    auto &primitive = *event.getIntersectionInfo()->primitive;
    auto uv = pointOnTriangle(primitive.textCoord[0], primitive.textCoord[1], primitive.textCoord[2],
                              event.uv().x(), event.uv().y());
    auto normal = bump.sample(uv);
    Vec3f Ns(normal.x(), normal.z(), normal.y());
    Ns -= Vec3f{0.5f, 0.5f, 0.5f};
    Ns *= 2;
    return Ns;
}

