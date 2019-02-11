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
            std::sqrt(std::max((Float) 0, (Float) 1 - cosThetaI * cosThetaI));
    Float tanThetaI = sinThetaI / cosThetaI;
    Float cotThetaI = 1 / tanThetaI;

    /* Search interval -- everything is parameterized
       in the Miyuki::erf() domain */
    Float a = -1, c = Miyuki::erf(cotThetaI);
    Float sample_x = std::max(U1, (Float) 1e-6f);

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
    *slope_y = erfInv(2.0f * std::max(U2, (Float) 1e-6f) - 1.0f);

//    CHECK(!std::isinf(*slope_x));
//    CHECK(!std::isnan(*slope_x));
//    CHECK(!std::isinf(*slope_y));
//    CHECK(!std::isnan(*slope_y));
}

static Vec3f BeckmannSample(const Vec3f &wi, Float alpha_x, Float alpha_y,
                            Float U1, Float U2) {
    // 1. stretch wi
    Vec3f wiStretched =
            Vec3f(alpha_x * wi.x(), wi.y(),  alpha_y *wi.z()).normalized();

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
    return Vec3f(-slope_x, 1.f, -slope_y).normalized();
}
// end pbrt


Miyuki::Float Miyuki::BeckmannDistribution::D(const Miyuki::Vec3f &wh) const {
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
    if (!sampleVisible) {
        // Sample full distribution of normals for Beckmann distribution

        // Compute $\tan^2 \theta$ and $\phi$ for Beckmann distribution sample
        Float tan2Theta, phi;
        if (alphaX == alphaY) {
            Float logSample = std::log(1 - u[0]);
            tan2Theta = -alphaX * alphaX * logSample;
            phi = u[1] * 2 * PI;
        } else {
            // Compute _tan2Theta_ and _phi_ for anisotropic Beckmann
            // distribution
            Float logSample = std::log(1 - u[0]);
            phi = std::atan(alphaY / alphaX *
                            std::tan(2 * PI * u[1] + 0.5f * PI));
            if (u[1] > 0.5f) phi += PI;
            Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
            Float alphax2 = alphaX * alphaX, alphay2 = alphaY * alphaY;
            tan2Theta = -logSample /
                        (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
        }

        // Map sampled Beckmann angles to normal direction _wh_
        Float cosTheta = 1 / std::sqrt(1 + tan2Theta);
        Float sinTheta = std::sqrt(std::max((Float) 0, 1 - cosTheta * cosTheta));
        Vec3f wh = sphericalDirection(sinTheta, cosTheta, phi);
        if (!sameHemisphere(wo, wh)) wh = -1 * wh;
        return wh;
    } else {
        // Sample visible area of normals for Beckmann distribution
        Vec3f wh;
        bool flip = wo.y() < 0;
        wh = BeckmannSample(flip ? -1 * wo : wo, alphaX, alphaY, u[0], u[1]);
        if (flip) wh = -1 * wh;
        return wh;
    }
}

static void TrowbridgeReitzSample11(Float cosTheta, Float U1, Float U2,
                                    Float *slope_x, Float *slope_y) {
    // special case (normal incidence)
    if (cosTheta > .9999) {
        Float r = sqrt(U1 / (1 - U1));
        Float phi = 6.28318530718 * U2;
        *slope_x = r * cos(phi);
        *slope_y = r * sin(phi);
        return;
    }

    Float sinTheta =
            std::sqrt(std::max((Float) 0, (Float) 1 - cosTheta * cosTheta));
    Float tanTheta = sinTheta / cosTheta;
    Float a = 1 / tanTheta;
    Float G1 = 2 / (1 + std::sqrt(1.f + 1.f / (a * a)));

    // sample slope_x
    Float A = 2 * U1 / G1 - 1;
    Float tmp = 1.f / (A * A - 1.f);
    if (tmp > 1e10) tmp = 1e10;
    Float B = tanTheta;
    Float D = std::sqrt(
            std::max(Float(B * B * tmp * tmp - (A * A - B * B) * tmp), Float(0)));
    Float slope_x_1 = B * tmp - D;
    Float slope_x_2 = B * tmp + D;
    *slope_x = (A < 0 || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

    // sample slope_y
    Float S;
    if (U2 > 0.5f) {
        S = 1.f;
        U2 = 2.f * (U2 - .5f);
    } else {
        S = -1.f;
        U2 = 2.f * (.5f - U2);
    }
    Float z =
            (U2 * (U2 * (U2 * 0.27385f - 0.73369f) + 0.46341f)) /
            (U2 * (U2 * (U2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
    *slope_y = S * z * std::sqrt(1.f + *slope_x * *slope_x);

//    CHECK(!std::isinf(*slope_y));
//    CHECK(!std::isnan(*slope_y));
}

static Vec3f TrowbridgeReitzSample(const Vec3f &wi, Float alpha_x,
                                   Float alpha_y, Float U1, Float U2) {
    // 1. stretch wi
    Vec3f wiStretched = (Vec3f(alpha_x * wi.x(), wi.y(), alpha_y * wi.z())).normalized();

    // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
    Float slope_x, slope_y;
    TrowbridgeReitzSample11(cosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

    // 3. rotate
    Float tmp = cosPhi(wiStretched) * slope_x - sinPhi(wiStretched) * slope_y;
    slope_y = sinPhi(wiStretched) * slope_x + cosPhi(wiStretched) * slope_y;
    slope_x = tmp;

    // 4. unstretch
    slope_x = alpha_x * slope_x;
    slope_y = alpha_y * slope_y;

    // 5. compute normal
    return (Vec3f(-slope_x, 1.0f, -slope_y).normalized());
}

Float TrowbridgeReitzDistribution::D(const Vec3f &wh) const {
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
    Vec3f wh;
    if (!sampleVisible) {
        Float cosTheta = 0, phi = (2 * PI) * u[1];
        if (alphaX == alphaY) {
            Float tanTheta2 = alphaX * alphaX * u[0] / (1.0f - u[0]);
            cosTheta = 1 / std::sqrt(1 + tanTheta2);
        } else {
            phi =
                    std::atan(alphaY / alphaX * std::tan(2 * PI * u[1] + .5f * PI));
            if (u[1] > .5f) phi += PI;
            Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
            const Float alphax2 = alphaX * alphaX, alphay2 = alphaY * alphaY;
            const Float alpha2 =
                    1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
            Float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
            cosTheta = 1 / std::sqrt(1 + tanTheta2);
        }
        Float sinTheta =
                std::sqrt(std::max((Float) 0., (Float) 1. - cosTheta * cosTheta));
        wh = sphericalDirection(sinTheta, cosTheta, phi);
        if (!sameHemisphere(wo, wh)) wh = -1 * wh;
    } else {
        bool flip = wo.y() < 0;
        wh = TrowbridgeReitzSample(flip ? -1 * wo : wo, alphaX, alphaY, u[0], u[1]);
        if (flip) wh = -1 * wh;
    }
    return wh;
}

Float MicrofacetDistribution::Pdf(const Vec3f &wo, const Vec3f &wh) const {
    if (sampleVisible)
        return D(wh) * G1(wo) * Vec3f::absDot(wo, wh) / absCosTheta(wo);
    else
        return D(wh) * absCosTheta(wh);
}
