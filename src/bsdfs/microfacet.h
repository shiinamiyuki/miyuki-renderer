//
// Created by Shiina Miyuki on 2019/3/8.
//

#ifndef MIYUKI_MICROFACET_H
#define MIYUKI_MICROFACET_H

#include "bsdf.h"

namespace Miyuki {

    enum class MicrofacetModel {
        beckmann,
        ggx
    };

    class MicrofacetDistribution {
    public:
        MicrofacetModel model;
        Float alphaX, alphaY;

        MicrofacetDistribution(MicrofacetModel model, Float alphaX, Float alphaY)
                : model(model), alphaX(alphaX), alphaY(alphaY) {}

        // measure of invisible microfacet
        Float lambda(const Vec3f &w) const {
            switch (model) {
                case MicrofacetModel::beckmann: {
                    Float absTanTheta = std::abs(TanTheta(w));
                    if (std::isinf(absTanTheta)) return 0.0f;
                    Float alpha = std::sqrt(Cos2Phi(w) * alphaX * alphaX +
                                            Sin2Phi(w) * alphaY * alphaY);
                    Float a = 1 / (alpha * absTanTheta);
                    if (a >= 1.6f)
                        return 0;
                    return (1 - 1.259f * a + 0.396f * a * a) /
                           (3.535f * a + 2.181f * a * a);
                }
                default:
                    throw NotImplemented();
            }
        }

        Float D(const Vec3f &wh) const {
            switch (model) {
                case MicrofacetModel::beckmann: {
                    Float tan2Theta = Tan2Theta(wh);
                    if (std::isinf(tan2Theta)) return 0.0f;
                    Float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
                    return std::exp(-tan2Theta * (Cos2Phi(wh) / (alphaX * alphaX) +
                                                  Sin2Phi(wh) / (alphaY * alphaY))) /
                           (PI * alphaX * alphaY * cos4Theta);
                }
                default:
                    throw NotImplemented();
            }
        }

        Float G1(const Vec3f &w) const {
            return 1.0f / (1.0f + lambda(w));
        }

        Float G(const Vec3f &wo, const Vec3f &wi) const {
            return 1.0f / (1.0f + lambda(wo) + lambda(wi));
        }

        Float pdf(const Vec3f &wo, const Vec3f &wh) const {
            return D(wh) / AbsCosTheta(wh);
        }

        Vec3f sampleWh(const Vec3f &wo, const Point2f &u) const {
            switch (model) {
                case MicrofacetModel::beckmann: {
                    Float phi, tan2Theta;
                    if (alphaX == alphaY) {
                        phi = u[0] * 2 * PI;
                        Float logU = std::log(1 - u[1]);
                        if (std::isinf(logU)) {
                            logU = 0.0f;
                        }
                        tan2Theta = -alphaX * alphaX * logU;
                    } else {
                        // ???
                        Float logSample = std::log(u[0]);
                        phi = std::atan(alphaY / alphaX *
                                        std::tan(2 * PI * u[1] + 0.5f * PI));
                        if (u[1] > 0.5f)
                            phi += PI;
                        Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
                        Float alphaX2 = alphaX * alphaX, alphaY2 = alphaY * alphaY;
                        tan2Theta = -logSample /
                                    (cosPhi * cosPhi / alphaX2 + sinPhi * sinPhi / alphaY2);
                    }
                    Float cosTheta = std::sqrt(1.0f / (1.0f + tan2Theta));
                    Float sinTheta = std::max(0.0f, sqrtf(1.0f - cosTheta * cosTheta));
                    auto wh = Vec3f(std::cos(phi) * sinTheta, cosTheta, std::sin(phi) * sinTheta).normalized();
                    if (!SameHemisphere(wo, wh)) wh *= -1;
                    return wh;
                }
                default:
                    throw NotImplemented();
            }
        }
    };

    class MicrofacetReflection : public BxDF {
        MicrofacetDistribution distribution;
        const Spectrum R;
        Fresnel *fresnel;
    public:
        MicrofacetReflection(const Spectrum &R,
                             MicrofacetDistribution distribution,
                             Fresnel *fresnel)
                : R(R), distribution(std::move(distribution)), fresnel(fresnel),
                  BxDF(BSDFLobe::glossy
                       | BSDFLobe::reflection) {}

        Float pdf(const ScatteringEvent &event) const override;

        Spectrum sample(ScatteringEvent &event) const override;

        Spectrum f(const ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_MICROFACET_H
