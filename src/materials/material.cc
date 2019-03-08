//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "material.h"
#include "bsdfs/lambertian.h"

namespace Miyuki {

    Float Material::emissionStrength() const {
        return emission.albedo.max();
    }

    void PBRMaterial::computeScatteringFunction(RenderContext &ctx, ScatteringEvent &event) const {
        BSDF *bsdf = ctx.arena->alloc<BSDF>();
        bsdf->add(ARENA_ALLOC(*ctx.arena, LambertianReflection)(info.kd.albedo));
        event.bsdf = bsdf;
    }
}