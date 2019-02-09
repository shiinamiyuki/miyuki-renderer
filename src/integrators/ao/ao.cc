//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "ao.h"
#include "../../core/scene.h"
#include "../../core/film.h"
#include "../../math/geometry.h"
#include "../../samplers/random.h"
#include "../../bsdfs/bsdf.h"

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
                ScatteringEvent event;
                Ray ray = ctx.primary;
                Intersection intersection(ray);
                IntersectionInfo info;
                if (!scene.intersect(ray, &info)) {
                    film.addSample(ctx.raster, Spectrum(0, 0, 0));
                    return;
                }
                event = makeScatteringEvent(ray, &info, ctx.sampler);
                ray = event.spawnRayLocal(cosineWeightedHemisphereSampling(ctx.sampler->nextFloat2D()));
                intersection = Intersection(ray);
                intersection.intersect(scene);
                if (!intersection.hit() || intersection.hitDistance() >= scene.option.aoDistance) {
                    Spectrum L(1,1,1);
                    film.addSample(ctx.raster, L);
                }else
                    film.addSample(ctx.raster, Spectrum(0, 0, 0));

            });
        });
        elapsed += t;
        fmt::print("iteration {} in {} secs, elapsed {}s, remaining {}s\n",
                   1 + i, t, elapsed, (double) (elapsed * N) / (i + 1) - elapsed);
    }
}
