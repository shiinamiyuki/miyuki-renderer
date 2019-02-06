//
// Created by Shiina Miyuki on 2019/2/3.
//

#ifndef MIYUKI_MATTE_H
#define MIYUKI_MATTE_H

#include "../core/material.h"
#include "../core/reflection.h"

namespace Miyuki {
    class MatteMaterial : public Material {
    public:
        MatteMaterial(const MaterialInfo&info):Material(info){}
        void computeScatteringFunctions(MemoryArena &arena, Interaction &interaction) const override;
    };
}
#endif //MIYUKI_MATTE_H
