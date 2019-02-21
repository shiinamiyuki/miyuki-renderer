//
// Created by Shiina Miyuki on 2019/2/9.
//

#include "bsdffactory.h"
#include "lambertian.h"
#include "oren-nayar.h"
#include "specular.h"
#include "fresnel.h"
#include "microfacet.h"
#include "mixed.h"
#include "../core/memory.h"
#include "../core/scene.h"

using namespace Miyuki;


std::shared_ptr<BSDF> BSDFFactory::operator()(const MaterialInfo &materialInfo) {
    std::shared_ptr<BSDF> bsdf;
    const auto &kd = materialInfo.kd;
    const auto &ks = materialInfo.ks;
    auto bump = materialInfo.bump;
    Fresnel *fresnel;
    fresnel = ARENA_ALLOC(scene->miscArena, FresnelPerfectSpecular)();
    if (kd.maxReflectance > 0) {
        if (materialInfo.roughness <= 1e-4f)
            bsdf = std::make_shared<LambertianBSDF>(kd, bump);
        else
            bsdf = std::make_shared<OrenNayarBSDF>(materialInfo.roughness, kd, bump);
    }
    if (ks.maxReflectance > 0) {
        std::shared_ptr<BSDF> bsdf2;
        Float alpha = 0;
        if (materialInfo.roughness > 1e-4f) {
            alpha = materialInfo.roughness;
        } else if (materialInfo.Ns >= 2) {
            alpha = powf(2 / (2 + materialInfo.Ns), 1.0f / 4.0f);
        }
        if (alpha <= 1e-4f) {
            bsdf2 = std::make_shared<SpecularBSDF>(fresnel, ks, bump);
        } else {
            Float alphaX, alphaY;
            alphaX = alphaY = BeckmannDistribution::roughnessToAlpha(alpha);
            bsdf2 = std::make_shared<MicrofacetBSDF<BeckmannDistribution>>(
                    BeckmannDistribution(alphaX, alphaY),
                    fresnel,
                    ks, bump);
        }
        CHECK(bsdf2);
        if (bsdf) {
            bsdf = std::make_shared<MixedBSDF>(bsdf, bsdf2, 1.0f, bump);
        } else {
            bsdf = bsdf2;
        }
    }
    if (!bsdf) {
        bsdf = std::make_shared<LambertianBSDF>(kd, bump);
    }
    bsdf->ka = materialInfo.ka;
    return bsdf;
}
