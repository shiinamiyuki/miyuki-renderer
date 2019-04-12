//
// Created by Shiina Miyuki on 2019/4/3.
//

#ifndef MIYUKI_DIRECT_HPP
#define MIYUKI_DIRECT_HPP

#include <integrators/integrator.h>
#include <bidir/vertex.h>

namespace Miyuki {
    class DirectLightingIntegrator : public SamplerIntegrator {
    public:
    protected:
        Spectrum Li(RenderContext &ctx, Scene &scene) override {
            using Bidir::Vertex, Bidir::SubPath;
            auto vertices = ctx.arena->alloc<Bidir::Vertex>(size_t(1 + 1));
            Spectrum beta(1, 1, 1);
            vertices[0] = Bidir::CreateCameraVertex(ctx.camera, ctx.raster, ctx.primary, 1.0f, beta);
            auto path = Bidir::RandomWalk(vertices + 1, ctx.primary, beta,
                                          1.0f, scene, ctx, 1, 1,
                                          TransportMode::radiance);
            Spectrum Li(0, 0, 0);
            bool specular = false;
            ctx.sampler->startDimension(4 + 4 * 1);
            Assert(path.N == 1);
            int depth = 0;

            Li += path[depth].beta * path[depth].Le(path[depth - 1]);

            if (!path[depth].isInfiniteLight())
                Li += path[depth].beta * importanceSampleOneLight(scene, ctx, *path[depth].event);
            specular = path[depth].delta;

            return Li;
        }

    public:
        DirectLightingIntegrator(int spp) {
            this->spp = spp;
            maxRayIntensity = 1000;
        }
    };
}
#endif //MIYUKI_DIRECT_HPP
