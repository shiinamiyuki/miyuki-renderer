//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "integrator.h"
#include "math/func.h"
#include "lights/light.h"
#include "thirdparty/hilbert/hilbert_curve.hpp"
#include "core/progress.h"
#include "math/sampling.h"
#include "core/scene.h"
#include "samplers/sobol.h"
#include "utils/thread.h"

namespace Miyuki {

    void Integrator::makeScatteringEvent(ScatteringEvent *event, RenderContext &ctx, Intersection *isct) {
        *event = ScatteringEvent(ctx.sampler, isct, nullptr);
        isct->primitive->material()->computeScatteringFunction(ctx, *event);
    }

    Spectrum Integrator::importanceSampleOneLight(Scene &scene, RenderContext &ctx, const ScatteringEvent &event) {
        Spectrum Ld(0, 0, 0);
        Float pdfLightChoice;
        Point2f lightSample = ctx.sampler->get2D();
        Point2f bsdfSample = ctx.sampler->get2D();
        auto light = scene.chooseOneLight(ctx.sampler, &pdfLightChoice);
        if (!light) {
            return {};
        }
        auto bsdf = event.bsdf;
        ScatteringEvent scatteringEvent = event;
        // sample light source
        {
            Vec3f wi;
            Float lightPdf;
            VisibilityTester tester;
            auto Li = light->sampleLi(lightSample, *event.getIntersection(), &wi, &lightPdf, &tester);
            lightPdf *= pdfLightChoice;

            if (lightPdf > 0 && !Li.isBlack()) {
                scatteringEvent.wiW = wi;
                scatteringEvent.wi = scatteringEvent.worldToLocal(wi);
                Spectrum f = bsdf->f(scatteringEvent) * Vec3f::absDot(wi, event.Ns());

                Float scatteringPdf = bsdf->pdf(scatteringEvent);
                if (!f.isBlack() && tester.visible(scene)) {
                    Float weight = PowerHeuristics(lightPdf, scatteringPdf);
                    Ld += f * Li * weight / lightPdf;
                }
            }
        }
        // sample brdf
        {
            scatteringEvent.u = bsdfSample;
            Spectrum f = bsdf->sample(scatteringEvent);
            const auto wi = scatteringEvent.wiW;
            f *= Vec3f::absDot(scatteringEvent.Ns(), wi);
            Float scatteringPdf = scatteringEvent.pdf;
            bool sampledSpecular = scatteringEvent.bsdfLobe.matchFlag(BSDFLobe::specular);
            if (!f.isBlack() && scatteringPdf > 0 && !sampledSpecular) {
                Ray ray = scatteringEvent.spawnRay(wi);
                Intersection isct;
                if (scene.intersect(ray, &isct) && isct.primitive->light) {
                    light = isct.primitive->light;
                    Float lightPdf = light->pdfLi(*event.getIntersection(), wi) * scene.pdfLightChoice(light);
                    auto Li = isct.Le(-1 * wi);
                    if (lightPdf > 0) {
                        Float weight = PowerHeuristics(scatteringPdf, lightPdf);
                        Ld += f * Li * weight / scatteringPdf;
                    }
                }
            }
        }
        return Ld;
    }

    void SamplerIntegrator::render(Scene &scene) {
        auto &film = *scene.film;
        Point2i nTiles = film.imageDimension() / TileSize + Point2i{1, 1};

        int M = std::ceil(std::log2(std::max(nTiles.x(), nTiles.y())));
        std::mutex mutex;
        std::vector<Point2f> hilbertMapping;
        for (int i = 0; i < pow(2, M + M); i++) {
            int tx, ty;
            ::d2xy(M, i, tx, ty);
            if (tx >= nTiles.x() || ty >= nTiles.y())
                continue;
            hilbertMapping.emplace_back(tx, ty);
        }

        ProgressReporter reporter(hilbertMapping.size(), [&](int cur, int total) {
            if (spp > 16) {
                if (cur % 16 == 0) {
                    std::lock_guard<std::mutex> lockGuard(mutex);
                    if (reporter.count() % 16 == 0) {
                        fmt::print("Rendered tiles: {}/{} Elapsed:{} Remaining:{}\n",
                                   cur,
                                   total, reporter.elapsedSeconds(), reporter.estimatedTimeToFinish());
                        scene.update();
                    }
                }
            }
        });
        std::vector<Seed> seeds(Thread::pool->numThreads());
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        Thread::ParallelFor(0u, hilbertMapping.size(), [&](uint32_t idx, uint32_t threadId) {

            int tx, ty;
            tx = hilbertMapping[idx].x();
            ty = hilbertMapping[idx].y();
            for (int i = 0; i < TileSize; i++) {
                for (int j = 0; j < TileSize; j++) {
                    if (!scene.processContinuable()) {
                        return;
                    }
                    int x = tx * TileSize + i;
                    int y = ty * TileSize + j;
                    if (x >= film.width() || y >= film.height())
                        continue;
                    auto raster = Point2i{x, y};
                    RandomSampler sampler(&seeds[threadId]);

                    for (int s = 0; s < spp; s++) {
                        // keeps minimum mem usage for cache efficiency
                        // should we do this?
                        arenas[threadId].reset();
                        auto ctx = scene.getRenderContext(raster, &arenas[threadId], &sampler);
                        auto Li = removeNaNs(L(ctx, scene));
                        Li = clampRadiance(Li, maxRayIntensity);
                        film.addSample({x, y}, Li, ctx.weight);
                    }
                }
            }
            reporter.update();
        });
        scene.update();
    }
}