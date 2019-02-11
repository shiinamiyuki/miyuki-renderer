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
    int32_t maxDepth = scene.option.maxDepth;
    bool showAL = scene.option.showAmbientLight;
    bool specular = false;
    IntersectionInfo info;
    ScatteringEvent event;
    for (int32_t depth = 0; depth < maxDepth; depth++) {
        if (!scene.intersect(ray, &info)) {
            if (!showAL || depth == 0) {
                L += scene.ambientLight;
            }
            break;
        }
        event = makeScatteringEvent(ray, &info, ctx.sampler);
        if (specular || depth == 0) {
            L += beta * event.Le(-1 * ray.d);
        }
        auto f = info.bsdf->sample(event);
        if (event.pdf <= 0) {
            break;
        }
        auto direct = importanceSampleOneLight(scene, ctx, event);
        specular = ((int) event.sampledType & (int) BSDFType::specular) != 0;

        L += beta * direct;
        beta *= f * Vec3f::absDot(event.wiW, info.normal) / event.pdf;
        ray = event.spawnRay(event.wiW);
        if (depth > scene.option.minDepth) {
            if (ctx.sampler->nextFloat() < beta.max()) {
                beta /= beta.max();
            } else {
                break;
            }
        }
    }
    L = clampRadiance(removeNaNs(L), 10);
    return L;
}


Spectrum PathTracer::importanceSampleOneLight(Scene &scene,
                                              RenderContext &ctx,
                                              ScatteringEvent &event,
                                              bool specular) {
    Spectrum Ld;
    auto light = scene.chooseOneLight(*ctx.sampler);
    ScatteringEvent scatteringEvent = event;
    auto bsdf = event.getIntersectionInfo()->bsdf;
    {
        Vec3f wi;
        VisibilityTester tester;
        Float lightPdf = 0, scatteringPdf = 0;
        auto Li = light->sampleLi(ctx.sampler->nextFloat2D(), *event.getIntersectionInfo(), &wi, &lightPdf, &tester);
        bool occlude = Vec3f::dot(wi, event.getIntersectionInfo()->Ng) < 0;
        scatteringEvent.wiW = wi;
        scatteringEvent.wi = scatteringEvent.worldToLocal(scatteringEvent.wiW);
        // sample light source
        if (lightPdf > 0 && !Li.isBlack()) {
            Spectrum f;
            f = bsdf->eval(scatteringEvent) * Vec3f::dot(wi, event.getIntersectionInfo()->normal);
            scatteringPdf = bsdf->pdf(event.wo, scatteringEvent.wi);
            if (!f.isBlack() && !occlude && tester.visible(scene)) {
                if (light->isDeltaLight()) {
                    Ld += f * Li / lightPdf;
                } else {
                    Float weight = powerHeuristic(1, lightPdf, 1, scatteringPdf);
                    Ld += f * Li * weight / lightPdf;

                }
            }
        }
    }
    // sample brdf
    if (!light->isDeltaLight()) {
        Spectrum f;
        bool sampledSpecular;
        f = bsdf->sample(scatteringEvent);
        Float scatteringPdf = scatteringEvent.pdf;
        Vec3f wi = scatteringEvent.wiW;
        Float lightPdf = 0;
        f *= Vec3f::dot(wi, event.getIntersectionInfo()->normal);
        sampledSpecular = ((int) event.sampledType & (int) BSDFType::specular) != 0;
        if (!f.isBlack() && scatteringPdf > 0) {
            Float weight;
            if (!sampledSpecular) {
                lightPdf = light->pdfLi(*event.getIntersectionInfo(), wi);
                if (lightPdf <= 0)
                    return Ld;
                weight = powerHeuristic(1, scatteringPdf, 1, lightPdf);
                Ray ray = event.spawnRay(wi);
                IntersectionInfo lightIntersect;
                Spectrum Li;
                if (scene.intersect(ray, &lightIntersect)) {
                    if (lightIntersect.primitive.raw() == light->getPrimitive()) {
                        Li = lightIntersect.Le(-1 * wi);
                    } else {
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