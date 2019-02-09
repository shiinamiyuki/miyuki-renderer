//
// Created by Shiina Miyuki on 2019/2/9.
//
#include "bsdf.h"
#include "oren-nayar.h"

using namespace Miyuki;

Spectrum Miyuki::OrenNayarBSDF::brdf(const ScatteringEvent &event) const {
    const auto &wi = event.wi;
    const auto &wo = event.wo;
    Float sinThetaI = sinTheta(wi);
    Float sinThetaO = sinTheta(wo);
    Float maxCos = 0.0f;
    if (sinThetaI > 1e-4f && sinThetaO > 1e-4f) {
        Float sinPhiI = sinPhi(wi), cosPhiI = cosPhi(wi);
        Float sinPhiO = sinPhi(wo), cosPhiO = cosPhi(wo);
        Float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max(0.0f, dCos);
    }
    Float sinAlpha, tanBeta;
    if (absCosTheta(wi) > absCosTheta(wo)) {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / absCosTheta(wi);
    } else {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / absCosTheta(wo);
    }
    return Spectrum(evalAlbedo(event) * INVPI * (A + B * maxCos * sinAlpha * tanBeta));
}
