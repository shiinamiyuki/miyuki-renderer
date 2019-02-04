//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "pathtracer.h"
#include "../core/scene.h"
#include "../core/sampler.h"
#include "../core/film.h"
#include "../core/geometry.h"
#include "../sampler/random.h"

using namespace Miyuki;

void PathTracer::render(Scene &scene) {
    fmt::print("Rendering\n");
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int N = scene.option.samplesPerPixel;
    int sleepTime = scene.option.sleepTime;
    double elapsed = 0;
    for (int i = 0; i < N; i++) {
        auto t = runtime([&]() {
            iteration(scene);
            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
        });
        elapsed += t;
        fmt::print("iteration {} in {} secs, elapsed {}s, remaining {}s\n",
                   1 + i, t, elapsed, (double) (elapsed * N) / (i + 1) - elapsed);
    }
}

void PathTracer::iteration(Scene &scene) {
    //TODO: refactoring, handle sampling based on BxDF tags ?
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    scene.foreachPixel([&](RenderContext & ctx) {
        film.addSample(ctx.raster, render(ctx.raster, ctx, scene));
    });
}

Spectrum PathTracer::render(const Point2i &, RenderContext &ctx, Scene &scene) {
    Sampler &randomSampler = *ctx.sampler;
    Ray ray = ctx.primary;
    Spectrum radiance;
    Vec3f throughput(1, 1, 1);
    Float weightLight = 1;
    int maxDepth = scene.option.maxDepth;
    bool showAL = scene.option.showAmbientLight;
    bool specular = false;
    for (int depth = 0; depth < maxDepth; depth++) {
        Intersection intersection(ray);
        intersection.intersect(scene);
        if (!intersection.hit()) {
            if (showAL || depth != 0)
                radiance += throughput * scene.ambientLight;
            break;
        }
        ray.o += ray.d * intersection.hitDistance();
        Interaction interaction;
        scene.fetchInteraction(intersection, &interaction);
        auto &primitive = *interaction.primitive;
        auto &material = *interaction.material;
        Vec3f wi;

        ray.d = wi;
        if(depth > scene.option.minDepth) {
            if (randomSampler.nextFloat() < throughput.max()) {
                throughput /= throughput.max();
            } else {
                break;
            }
        }
    }
    return radiance;
}
