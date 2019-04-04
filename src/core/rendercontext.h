//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_RENDERCONTEXT_H
#define MIYUKI_RENDERCONTEXT_H

#include "miyuki.h"
#include "ray.h"
#include "core/memory.h"
#include "cameras/camera.h"
#include "samplers/sampler.h"

namespace Miyuki {
    struct RenderContext {
        RayDifferential primary;
        Camera *camera;
        MemoryArena *arena;
        Sampler *sampler;
        Point2f raster;
        Float weight;

        RenderContext(const Point2f &raster,
                      const RayDifferential &primary,
                      Camera *camera,
                      MemoryArena *arena,
                      Sampler *sampler,
                      Float weight = 1) :
                raster(raster), primary(primary), camera(camera), arena(arena), sampler(sampler),
                weight(weight) {

        }
    };
}
#endif //MIYUKI_RENDERCONTEXT_H
