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
    int32_t N = scene.option.samplesPerPixel;
    int32_t sleepTime = scene.option.sleepTime;
    double elapsed = 0;
    for (int32_t i = 0; i < N; i++) {
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
    scene.foreachPixel([&](RenderContext &ctx) {
        film.addSample(ctx.raster, render(ctx.raster, ctx, scene));
    });
}

Spectrum PathTracer::render(const Point2i &, RenderContext &ctx, Scene &scene) {
    Sampler &randomSampler = *ctx.sampler;
    Ray ray = ctx.primary;
    Spectrum L;
    Vec3f beta(1, 1, 1);
    Float misWeight = 1;
    int32_t maxDepth = scene.option.maxDepth;
    bool showAL = scene.option.showAmbientLight;
    bool specular = false;
    for (int32_t depth = 0; depth < maxDepth; depth++) {
        Interaction interaction;
        if (!scene.intersect(ray, &interaction)) {
            if (showAL || depth == 0)
                L += beta * scene.ambientLight;
            break;
        }
        L += beta * interaction.Le(-1 * ray.d) * misWeight;
        interaction.computeScatteringFunctions(ctx.arena);
        Float lightPdf;
        auto direct = importanceSampleOneLight(interaction, scene, ctx, &lightPdf);
        Vec3f wi;
        Float brdfPdf = 0;
        BxDFType flags;
        auto f = interaction.bsdf->sampleF(interaction.wo, &wi, ctx.sampler->nextFloat2D(),
                                           &brdfPdf, BxDFType::all, &flags);
        specular = ((int) flags & (int) BxDFType::specular) != 0;
        if (brdfPdf == 0)
            break;
        if (lightPdf != 0 && !specular) {
            misWeight = 1 / (lightPdf + brdfPdf);
            L += beta * direct * misWeight;
        } else {
            misWeight = 1;
        }

        beta *= f * Vec3f::absDot(wi, interaction.normal) / brdfPdf;

        ray = interaction.spawnRay(wi);
        if (depth > scene.option.minDepth) {
            if (randomSampler.nextFloat() < beta.max()) {
                beta /= beta.max();
            } else {
                break;
            }
        }
    }
    L = clampRadiance(removeNaNs(L), 10);
    return L;
}

Spectrum PathTracer::importanceSampleOneLight(const Interaction &interaction, const Scene &scene, RenderContext &ctx,
                                              Float *pdf) {
    auto light = scene.chooseOneLight(*ctx.sampler);
    Vec3f wi;
    VisibilityTester visibilityTester;
    auto Li = light->sampleLi(ctx.sampler->nextFloat2D(), interaction, &wi, pdf, &visibilityTester);
    bool occlude = Vec3f::dot(wi, interaction.Ng) < 0;
    auto brdf = interaction.bsdf->f(interaction.wo, wi);
    if (brdf.max() > 0.0f) {
        if (!occlude && visibilityTester.visible(scene)) {
            Li *= brdf * std::max(0.0f, Vec3f::dot(interaction.normal, wi));
        } else { return {}; }
    } else {
        *pdf = 0;
        return {};
    }
    return Li;
}
