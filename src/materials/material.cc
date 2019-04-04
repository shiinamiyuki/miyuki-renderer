//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "material.h"
#include "bsdfs/lambertian.h"
#include "bsdfs/specular.h"
#include "bsdfs/microfacet.h"
#include <bsdfs/toon.h>

namespace Miyuki {

    Float Material::emissionStrength() const {
        return emission.albedo.max() * emission.multiplier;
    }

    void PBRMaterial::computeScatteringFunction(RenderContext &ctx, ScatteringEvent &event) const {
        BSDF *bsdf = ctx.arena->alloc<BSDF>();
        if (info.kd.albedo.max() > 1e-5f)
            bsdf->add(ARENA_ALLOC(*ctx.arena, LambertianReflection)(info.kd.evalUV(event.textureUV())));
        if (info.ks.albedo.max() > 1e-5f) {
            if (info.roughness < 0.001f) {
                if (info.Tr < 0.9)
                    bsdf->add(ARENA_ALLOC(*ctx.arena, SpecularReflection)(info.ks.evalUV(event.textureUV())));
                else
                    bsdf->add(ARENA_ALLOC(*ctx.arena, SpecularTransmission)(info.ks.evalUV(event.textureUV()),
                                                                            1.0f, info.Ni));
            } else {
                Float ax, ay;
                ax = ay = info.roughness * info.roughness;
                bsdf->add(ARENA_ALLOC(*ctx.arena, MicrofacetReflection)(info.ks.evalUV(event.textureUV()),
                                                                        MicrofacetDistribution(
                                                                                MicrofacetModel::beckmann, ax, ay),
                                                                        ARENA_ALLOC(*ctx.arena,
                                                                                    PerfectSpecularFresnel)()));
            }
        }
        event.bsdf = bsdf;
    }

    Json::JsonObject PBRMaterial::toJson() const {
        return std::move(IO::serialize(info));
    }
}