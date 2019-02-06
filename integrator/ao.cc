//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "ao.h"
#include "../core/scene.h"
#include "../core/sampler.h"
#include "../core/film.h"
#include "../core/geometry.h"
#include "../sampler/random.h"

using namespace Miyuki;

void AOIntegrator::render(Scene &scene) {
    fmt::print("Rendering AO\n");
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int32_t N = scene.option.samplesPerPixel;
    double elapsed = 0;
    for (int32_t i = 0; i < N; i++) {
        auto t = runtime([&]() {
            scene.foreachPixel([&](RenderContext &ctx) {
                Ray ray = ctx.primary;
                Intersection intersection(ray);
                intersection.intersect(scene);
                if (!intersection.hit()) {
                    film.addSample(ctx.raster, Spectrum(0, 0, 0));
                    return;
                }
                ray.o += ray.d * intersection.hitDistance();
                Interaction* interaction = ctx.arena.alloc<Interaction>();
                scene.fetchInteraction(intersection, interaction);
                ray.d = cosineWeightedHemisphereSampling(interaction->normal,
                       ctx.sampler->nextFloat(),ctx.sampler->nextFloat());
                intersection = Intersection(ray);
                intersection.intersect(scene);
                if (!intersection.hit() || intersection.hitDistance() >= scene.option.aoDistance)
                    film.addSample(ctx.raster, Spectrum(1, 1, 1));
                else
                    film.addSample(ctx.raster, Spectrum(0, 0, 0));
            });
        });
        elapsed += t;
        fmt::print("iteration {} in {} secs, elapsed {}s, remaining {}s\n",
                   1 + i, t, elapsed, (double) (elapsed * N) / (i + 1) - elapsed);
    }
}
