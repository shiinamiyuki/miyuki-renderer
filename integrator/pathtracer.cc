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
    scene.foreachPixel([&](const Point2i &pos) {
        auto ctx = scene.getRenderContext(pos);
        film.addSample(pos, render(pos, ctx, scene));
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
        scene.fetchInteraction(intersection, makeRef(&interaction));
        auto &primitive = *interaction.primitive;
        auto &material = *interaction.material;
        Vec3f wi;
        Float pdf;
        BxDFType sampledType;
        auto sample = material.sampleF(randomSampler, interaction, ray.d, &wi, &pdf, BxDFType::all, &sampledType);
        if (sampledType == BxDFType::none) { break; }
        if (sampledType == BxDFType::emission) {
            radiance += throughput * sample / pdf * clamp<Float>(weightLight, 0, 1);
            if (!specular) {
                radiance = clampRadiance(radiance, 5);
            }
            break;
        } else if (sampledType == BxDFType::diffuse) {
            // TODO: should we optimize the code to cancel the cosine term?
            //throughput *= sample * Vec3f::dot(interaction.normal, wi) / pdf;
            throughput *= sample;
            auto light = scene.chooseOneLight(randomSampler);
            Vec3f L;
            Float lightPdf;
            VisibilityTester visibilityTester;
            auto ka = light->sampleLi(randomSampler.nextFloat2D(),
                                      interaction, &L, &lightPdf, &visibilityTester);
            Float cosWi = -Vec3f::dot(L, interaction.normal);
            Float brdf = material.f(sampledType, interaction, ray.d, -1 * L);
            // balanced heuristics
            auto misPdf = pdf + lightPdf;
            weightLight = 1 / misPdf;
            if (brdf > EPS && lightPdf > 0 && cosWi > 0 && visibilityTester.visible(scene)) {
                radiance += throughput * ka / misPdf * cosWi;
                // suppress fireflies
                //   radiance = clampRadiance(radiance, 4);
            }
            throughput *= Vec3f::dot(interaction.normal, wi) / pdf;
        } else {
            assert(hasBxDFType(sampledType, BxDFType::specular));
            specular = true;
            if (hasBxDFType(sampledType, BxDFType::glossy)) {
                throughput *= sample;
                auto light = scene.chooseOneLight(randomSampler);
                Vec3f L;
                Float lightPdf;
                VisibilityTester visibilityTester;
                auto ka = light->sampleLi(randomSampler.nextFloat2D(),
                                          interaction, &L, &lightPdf, &visibilityTester);
                Float cosWi = -Vec3f::dot(L, interaction.normal);
                Float brdf = material.f(sampledType, interaction, ray.d, -1 * L);
                auto misPdf = brdf + lightPdf;
                weightLight = 1 / misPdf;
                if (brdf > EPS && lightPdf > 0 && cosWi > 0 && visibilityTester.visible(scene)) {
                    radiance += brdf * throughput * ka / misPdf;
                    //  radiance = clampRadiance(radiance, 4);
                }
                throughput /= pdf;
            } else {
                weightLight = 1;
                throughput *= sample / pdf;
            }
        }
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
