//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "material.h"
#include "bsdfs/lambertian.h"
#include "bsdfs/specular.h"
#include "bsdfs/microfacet.h"
namespace Miyuki {

    Float Material::emissionStrength() const {
        return emission.albedo.max();
    }

    void PBRMaterial::computeScatteringFunction(RenderContext &ctx, ScatteringEvent &event) const {
        BSDF *bsdf = ctx.arena->alloc<BSDF>();
        if (info.kd.albedo.max() > 1e-5f)
            bsdf->add(ARENA_ALLOC(*ctx.arena, LambertianReflection)(info.kd.albedo));
        if (info.ks.albedo.max() > 1e-5f) {
            if(info.roughness < 0.001f)
                bsdf->add(ARENA_ALLOC(*ctx.arena, SpecularReflection)(info.ks.albedo));
            else{
                Float ax, ay;
                ax = ay = info.roughness * info.roughness;
                bsdf->add(ARENA_ALLOC(*ctx.arena, MicrofacetReflection)(info.ks.albedo,
                        MicrofacetDistribution(MicrofacetModel::beckmann, ax, ay),
                        ARENA_ALLOC(*ctx.arena, PerfectSpecularFresnel)()));
            }
        }
        event.bsdf = bsdf;
    }
}