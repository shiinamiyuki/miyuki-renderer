//
// Created by Shiina Miyuki on 2019/2/9.
//

#include "bsdffactory.h"
#include "lambertian.h"
#include "oren-nayar.h"
#include "specular.h"
#include "fresnel.h"
#include "microfacet.h"
#include "../core/memory.h"
#include "../core/scene.h"

using namespace Miyuki;

struct LevenshteinString : public std::string {
public:
    LevenshteinString(const std::string &s) : std::string(s) {}

    bool operator==(const char *s) const {
        auto dist = editDistance(*this, s, false);
        if (length() <= 5)
            return dist == 0;
        else
            return dist <= 3;
    }
};

std::shared_ptr<BSDF> BSDFFactory::operator()(const MaterialInfo &materialInfo) {
    std::shared_ptr<BSDF> bsdf;
    LevenshteinString bsdfType = materialInfo.bsdfType;
    const auto &kd = materialInfo.kd;
    const auto &ks = materialInfo.ks;
    LevenshteinString microfacetType = materialInfo.microfacetType;
    LevenshteinString fresnelType = materialInfo.fresnelType;
    Fresnel *fresnel = nullptr;
    if (fresnelType.empty() || fresnelType == "default") {
        fresnel = ARENA_ALLOC(scene->miscArena, FresnelPerfectSpecular)();
    }
    if (!fresnel) {
        fresnel = ARENA_ALLOC(scene->miscArena, FresnelPerfectSpecular)();
    }
    try {
        if (bsdfType == "default") {
            bsdf = std::make_shared<LambertianBSDF>(kd);
        } else if (bsdfType == "lambertian") {
            bsdf = std::make_shared<LambertianBSDF>(kd);
        } else if (bsdfType == "OrenNayar") {
            auto sigma = std::stof(materialInfo.parameters.at("sigma"));
            if (sigma > 1e-4f)
                bsdf = std::make_shared<OrenNayarBSDF>(sigma, kd);
            else
                bsdf = std::make_shared<LambertianBSDF>(kd);
        } else if (bsdfType == "microfacet") {
            Float alphaX, alphaY;

            if (microfacetType == "Beckmann") {
                alphaX = alphaY = BeckmannDistribution::roughnessToAlpha(materialInfo.glossiness);
                bsdf = std::make_shared<MicrofacetBSDF<BeckmannDistribution>>(
                        BeckmannDistribution(alphaX, alphaY),
                        fresnel,
                        ks);
            } else if (microfacetType == "TrowbridgeReitz" || microfacetType == "TR") {
                alphaX = alphaY = TrowbridgeReitzDistribution::roughnessToAlpha(materialInfo.glossiness);
                bsdf = std::make_shared<MicrofacetBSDF<TrowbridgeReitzDistribution>>(
                        TrowbridgeReitzDistribution(alphaX, alphaY),
                        fresnel,
                        ks);
            } else {
                fmt::print(stderr, "Unrecognized microfacet model: {}\n", microfacetType);
            }
        } else if (bsdfType == "specular" || bsdfType == "reflection") {
            //if (materialInfo.Tr <= 0) {
            bsdf = std::make_shared<SpecularBSDF>(fresnel, ks);
            // }
        } else {
            fmt::print(stderr, "Unrecognized bsdf type: {}\n", bsdfType);
            bsdf = std::make_shared<LambertianBSDF>(kd);
        }
    } catch (std::exception &e) {
        fmt::print(stderr, "Exception in BSDFFactory: {}\n", e.what());
        bsdf = std::make_shared<LambertianBSDF>(kd);
    }
    if (!bsdf) {
        fmt::print(stderr, "Failed to construct BSDF for parameters: {} {} {}\n", bsdfType, microfacetType,
                   fresnelType);
        bsdf = std::make_shared<LambertianBSDF>(kd);
    }
    bsdf->ka = materialInfo.ka;
    return bsdf;
}
