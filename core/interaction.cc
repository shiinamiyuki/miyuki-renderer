//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "interaction.h"

using namespace Miyuki;


Point2f Interaction::textureCoord() const {
    return pointOnTriangle(primitive->textCoord[0], primitive->textCoord[1], primitive->textCoord[2], uv.x(), uv.y());
}

void Interaction::computeScatteringFunctions(MemoryArena &arena) {
    material->computeScatteringFunctions(arena, *this);
}
