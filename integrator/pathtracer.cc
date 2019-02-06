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
        if (specular || depth == 0) {
            L += beta * interaction.Le(-1 * ray.d) * misWeight;
        }
        interaction.computeScatteringFunctions(ctx.arena);
        auto direct = importanceSampleOneLight(interaction, scene, ctx);
        Vec3f wi;
        Float brdfPdf = 0;
        BxDFType flags;
        auto f = interaction.bsdf->sampleF(interaction.wo, &wi, ctx.sampler->nextFloat2D(),
                                           &brdfPdf, BxDFType::all, &flags);
        specular = ((int) flags & (int) BxDFType::specular) != 0;
        if (brdfPdf == 0)
            break;
        L += beta * direct;

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

Spectrum PathTracer::importanceSampleOneLight(const Interaction &interaction, Scene &scene, RenderContext &ctx,
                                              bool specular) {
    Spectrum Ld;
    BxDFType flags = specular ? BxDFType::all : BxDFType((int) BxDFType::all & ~(int) BxDFType::specular);
    auto light = scene.chooseOneLight(*ctx.sampler);
    Vec3f wi;
    VisibilityTester visibilityTester;
    Float lightPdf = 0, scatteringPdf = 0;
    // sample light source
    auto Li = light->sampleLi(ctx.sampler->nextFloat2D(), interaction, &wi, &lightPdf, &visibilityTester);
    bool occlude = Vec3f::dot(wi, interaction.Ng) < 0;
    // TODO: where should I put `if(!occlude)` ?
    if (lightPdf > 0 && !Li.isBlack()) {
        Spectrum f;
        f = interaction.bsdf->f(interaction.wo, wi, flags) * Vec3f::dot(wi, interaction.normal);
        scatteringPdf = interaction.bsdf->Pdf(interaction.wo, wi, flags);
        if (!f.isBlack() && !occlude && visibilityTester.visible(scene)) {
            if (light->isDeltaLight()) {
                Ld += f * Li / lightPdf;
            } else {
                Float weight = powerHeuristic(1, lightPdf, 1, scatteringPdf);
                Ld += f * Li * weight / lightPdf;
            }
        }
    }
    // sample brdf
    if (!light->isDeltaLight()) {
        Spectrum f;
        BxDFType sampledType;
        bool sampledSpecular = false;
        f = interaction.bsdf->sampleF(interaction.wo, &wi, ctx.sampler->nextFloat2D(), &scatteringPdf, flags,
                                      &sampledType);
        f *= Vec3f::dot(wi, interaction.normal);
        sampledSpecular = ((int) sampledType & (int) BxDFType::specular) != 0;
        if (!f.isBlack() && scatteringPdf > 0) {
            Float weight;
            if (!sampledSpecular) {
                lightPdf = light->pdfLi(interaction, wi);
                if (lightPdf <= 0)
                    return Ld;
                weight = powerHeuristic(1, scatteringPdf, 1, lightPdf);
                Ray ray = interaction.spawnRay(wi);
                Interaction lightIntersect;
                Li = Spectrum{};
                if (scene.intersect(ray, &lightIntersect)) {
                    if (lightIntersect.primitive.raw() == light->getPrimitive()) {
                        Li = lightIntersect.Le(-1 * wi);
                    }else{
                        return Ld;
                    }
                }
                if (!Li.isBlack()) {
                    Ld += Li * f * weight / scatteringPdf;
                }
            }
        }
    }
    return Ld;
}
