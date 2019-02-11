//
// Created by Shiina Miyuki on 2019/2/2.
//

#include "microfacet.h"
#include "../math/func.h"
using namespace Miyuki;

// From pbrt
// https://github.com/mmp/pbrt-v3/blob/master/src/core/microfacet.cpp
// Microfacet Utility Functions
static void BeckmannSample11(Float cosThetaI, Float U1, Float U2,
                             Float *slope_x, Float *slope_y) {
    /* Special case (normal incidence) */
    if (cosThetaI > .9999) {
        Float r = std::sqrt(-std::log(1.0f - U1));
        Float sinPhi = std::sin(2 * PI * U2);
        Float cosPhi = std::cos(2 * PI * U2);
        *slope_x = r * cosPhi;
        *slope_y = r * sinPhi;
        return;
    }

    /* The original inversion routine from the paper contained
       discontinuities, which causes issues for QMC integration
       and techniques like Kelemen-style MLT. The following code
       performs a numerical inversion with better behavior */
    Float sinThetaI =
            std::sqrt(std::max((Float)0, (Float)1 - cosThetaI * cosThetaI));
    Float tanThetaI = sinThetaI / cosThetaI;
    Float cotThetaI = 1 / tanThetaI;

    /* Search interval -- everything is parameterized
       in the Miyuki::erf() domain */
    Float a = -1, c = Miyuki::erf(cotThetaI);
    Float sample_x = std::max(U1, (Float)1e-6f);

    /* Start with a good initial guess */
    // Float b = (1-sample_x) * a + sample_x * c;

    /* We can do better (inverse of an approximation computed in
     * Mathematica) */
    Float thetaI = std::acos(cosThetaI);
    Float fit = 1 + thetaI * (-0.876f + thetaI * (0.4265f - 0.0594f * thetaI));
    Float b = c - (1 + c) * std::pow(1 - sample_x, fit);

    /* Normalization factor for the CDF */
    static const Float SQRT_PI_INV = 1.f / std::sqrt(PI);
    Float normalization =
            1 /
            (1 + c + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

    int it = 0;
    while (++it < 10) {
        /* Bisection criterion -- the oddly-looking
           Boolean expression are intentional to check
           for NaNs at little additional cost */
        if (!(b >= a && b <= c)) b = 0.5f * (a + c);

        /* Evaluate the CDF and its derivative
           (i.e. the density function) */
        Float invErf = erfInv(b);
        Float value =
                normalization *
                (1 + b + SQRT_PI_INV * tanThetaI * std::exp(-invErf * invErf)) -
                sample_x;
        Float derivative = normalization * (1 - invErf * tanThetaI);

        if (std::abs(value) < 1e-5f) break;

        /* Update bisection intervals */
        if (value > 0)
            c = b;
        else
            a = b;

        b -= value / derivative;
    }

    /* Now convert back into a slope value */
    *slope_x = erfInv(b);

    /* Simulate Y component */
    *slope_y = erfInv(2.0f * std::max(U2, (Float)1e-6f) - 1.0f);

    assert(!std::isinf(*slope_x));
    assert(!std::isnan(*slope_x));
    assert(!std::isinf(*slope_y));
    assert(!std::isnan(*slope_y));
}

static Vec3f BeckmannSample(const Vec3f &wi, Float alpha_x, Float alpha_y,
                               Float U1, Float U2) {
    // 1. stretch wi
    Vec3f wiStretched =
           Vec3f(alpha_x * wi.x(), alpha_y * wi.y(), wi.z()).normalized();

    // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
    Float slope_x, slope_y;
    BeckmannSample11(cosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

    // 3. rotate
    Float tmp = cosPhi(wiStretched) * slope_x - sinPhi(wiStretched) * slope_y;
    slope_y = sinPhi(wiStretched) * slope_x + cosPhi(wiStretched) * slope_y;
    slope_x = tmp;

    // 4. unstretch
    slope_x = alpha_x * slope_x;
    slope_y = alpha_y * slope_y;

    // 5. compute normal
    return Vec3f(-slope_x, -slope_y, 1.f).normalized();
}
// end pbrt


Miyuki::Float Miyuki::BeckmannDistribution::D(const Miyuki::Vec3f &wh) {
    Float t2 = tan2Theta(wh);
    if (std::isinf(t2)) return 0.0;
    Float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
    return std::exp(-t2 * (cos2Phi(wh) / (alphaX * alphaX) +
                           sin2Phi(wh) / (alphaY * alphaY))) /
           (PI * alphaX * alphaY * cos4Theta);
}

Float BeckmannDistribution::lambda(const Vec3f &w) const {
    Float absTanTheta = std::abs(tanTheta(w));
    if (std::isinf(absTanTheta))return 0;
    Float alpha = std::sqrt(cos2Phi(w) * alphaX + alphaX + sin2Phi(w) * alphaY * alphaY);
    Float a = 1.0f / (alpha * absTanTheta);
    if (a >= 1.6f)return 0;
    return (1 - 1.259f * a + 0.396f * a * a) /
           (3.535f * a + 2.181f * a * a);
}

Vec3f BeckmannDistribution::sampleWh(const Vec3f &wo, const Point2f &u) const {
    return Vec3f();
}

Float TrowbridgeReitzDistribution::D(const Vec3f &wh) {
    Float t2 = tan2Theta(wh);
    if (std::isinf(t2)) return 0.0f;
    const Float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
    Float e = (cos2Phi(wh) / (alphaX * alphaX) +
               sin2Phi(wh) / (alphaY * alphaY)) * t2;
    return 1 / (PI * alphaX * alphaY * cos4Theta * (1 + e) * (1 + e));
}

Float TrowbridgeReitzDistribution::lambda(const Vec3f &w) const {
    Float absTanTheta = std::abs(tanTheta(w));
    if (std::isinf(absTanTheta))return 0;
    Float alpha = std::sqrt(cos2Phi(w) * alphaX + alphaX + sin2Phi(w) * alphaY * alphaY);
    Float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + std::sqrt(1.0f + alpha2Tan2Theta)) / 2;
}

Vec3f TrowbridgeReitzDistribution::sampleWh(const Vec3f &wo, const Point2f &u) const {
    return Vec3f();
}
