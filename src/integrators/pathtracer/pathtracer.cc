//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "pathtracer.h"
#include "../../core/scene.h"
#include "../../core/film.h"
#include "../../math/geometry.h"
#include "../../samplers/random.h"
#include "../../bsdfs/bsdf.h"
#include "../../lights/light.h"
#include "../../utils/stats.hpp"

using namespace Miyuki;
static DECLARE_STATS(uint32_t, pathCount);
static DECLARE_STATS(uint32_t, zeroPathCount);

void PathTracer::render(Scene &scene) {
    fmt::print("Rendering\n");
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int32_t N = scene.option.samplesPerPixel;
    int32_t sleepTime = scene.option.sleepTime;
    pathCount = 0;
    zeroPathCount = 0;
    double elapsed = 0;
    for (int32_t i = 0; i < N && scene.processContinuable(); i++) {
        auto t = runtime([&]() {
            iteration(scene);
            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
        });
        elapsed += t;
        fmt::print("iteration {} in {} secs, elapsed {}s, remaining {}s, zero-path: {}%\n",
                   1 + i, t, elapsed, (double) (elapsed * N) / (i + 1) - elapsed,
                   (double) zeroPathCount / (double) pathCount * 100);
        scene.update();
    }
}

void PathTracer::iteration(Scene &scene) {
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    scene.foreachPixel([&](RenderContext &ctx) {
        auto L = render(ctx.raster, ctx, scene);
        UPDATE_STATS(pathCount, 1);
        if (L.isBlack())
            UPDATE_STATS(zeroPathCount, 1);
        film.addSample(ctx.raster, L, ctx.filterWeight);
    });
}

Spectrum PathTracer::render(const Point2i &, RenderContext &ctx, Scene &scene) {
    Ray ray = ctx.primary;
    Spectrum L;
    Spectrum beta(1, 1, 1);
    int32_t maxDepth = scene.option.maxDepth;
    bool showAL = scene.option.showAmbientLight;
    bool specular = false;
    IntersectionInfo info;
    ScatteringEvent event;
    for (int32_t depth = 0; depth < maxDepth; depth++) {
        if (!scene.intersect(ray, &info)) {
            if (showAL || depth == 0) {
                L += beta * scene.infiniteLight->L();
            }
            break;
        }
        event = makeScatteringEvent(ray, &info, ctx.sampler);
        if (specular || depth == 0) {
            L += beta * event.Le(-1 * ray.d);
        }
        auto f = info.bsdf->sample(event);
        specular = ((int) event.sampledType & (int) BSDFType::specular) != 0;
        if (event.pdf <= 0) {
            break;
        }

        auto direct = importanceSampleOneLight(scene, ctx, event);
        if (!direct.isBlack())
            L += beta * direct;
        beta *= f * Vec3f::absDot(event.wiW, event.Ns) / event.pdf;
        ray = event.spawnRay(event.wiW);
        if (depth > scene.option.minDepth) {
            Float p = std::min(1.0f, beta.max());
            if (ctx.sampler->nextFloat() < p) {
                beta /= p;
            } else {
                break;
            }
        }
    }
//    L = clampRadiance(removeNaNs(L), scene.option.maxRayIntensity);
    return L;
}


