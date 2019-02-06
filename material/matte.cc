//
// Created by Shiina Miyuki on 2019/2/3.
//

#include "matte.h"

using namespace Miyuki;

void MatteMaterial::computeScatteringFunctions(MemoryArena &arena, Interaction &interaction) const {
    interaction.bsdf = ARENA_ALLOC(arena, BSDF)(interaction);
    auto R = materialInfo.kd.sample(interaction.textureCoord());
    interaction.bsdf->add(ARENA_ALLOC(arena, LambertianReflection)(R));
}
