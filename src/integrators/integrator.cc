//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "integrator.h"
#include "math/func.h"
#include "lights/light.h"
#include "core/progress.h"
#include "math/sampling.h"
#include "core/scene.h"
#include "samplers/sobol.h"
#include "utils/thread.h"

namespace Miyuki {
    void HilbertMapping(const Point2i &nTiles, std::vector<Point2f> &hilbertMapping) {
        int M = std::ceil(std::log2(std::max(nTiles.x(), nTiles.y())));

        for (int i = 0; i < pow(2, M + M); i++) {
            int tx, ty;
            ::d2xy(M, i, tx, ty);
            if (tx >= nTiles.x() || ty >= nTiles.y())
                continue;
            hilbertMapping.emplace_back(tx, ty);
        }
        uint32_t mid = (hilbertMapping.size() / 2);
        for (int i = 0; i < mid / 2; i++) {
            std::swap(hilbertMapping[i], hilbertMapping[mid - i - 1]);
        }
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
                if (scene.intersect(ray, &isct) && isct.primitive->light()) {
                    light = isct.primitive->light();
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
        if (progressive) {
            renderProgressive(scene);
            return;
        }
        auto &film = *scene.film;
        Point2i nTiles = film.imageDimension() / TileSize + Point2i{1, 1};


        std::mutex mutex;
        std::vector<Point2f> hilbertMapping;
        HilbertMapping(nTiles, hilbertMapping);

        ProgressReporter<uint32_t> reporter(hilbertMapping.size(), [&](uint32_t cur, uint32_t total) {
            if (spp > 16) {

                fmt::print("Rendered tiles: {}/{} Elapsed:{} Remaining:{}\n",
                           cur,
                           total, reporter.elapsedSeconds(), reporter.estimatedTimeToFinish());
                std::unique_lock<std::mutex> lockGuard(mutex, std::try_to_lock);
                if (lockGuard.owns_lock())
                    scene.update();
            }
        });
        std::vector<Seed> seeds(Thread::pool->numThreads());
        {
            std::random_device rd;
            std::uniform_int_distribution<Seed> dist(1, UINT64_MAX);
            for (auto &i:seeds) {
                i = dist(rd);
            }
        }
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        Thread::ParallelFor(0u, hilbertMapping.size(), [&](uint32_t idx, uint32_t threadId) {
            int tx, ty;
            tx = hilbertMapping[idx].x();
            ty = hilbertMapping[idx].y();
            Point2i tilePos(tx, ty);
            tilePos *= TileSize;
            Bound2i tileBound(tilePos, tilePos + Point2i{TileSize, TileSize});
            auto tile = film.getFilmTile(tileBound);
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
                    SobolSampler sampler(&seeds[threadId]);

                    for (int s = 0; s < spp; s++) {
                        // keeps minimum mem usage for cache efficiency
                        // should we do this?
                        arenas[threadId].reset();
                        auto ctx = scene.getRenderContext(raster, &arenas[threadId], &sampler);
                        auto Li = removeNaNs(this->Li(ctx, scene));
                        Li = clampRadiance(Li, maxRayIntensity);
                        tile->addSample(ctx.raster, Li, ctx.weight);
                    }
                }
            }
            film.mergeFilmTile(*tile);
            reporter.update();
        });
        scene.update();

    }

    void SamplerIntegrator::renderProgressive(Scene &scene) {
        ProgressReporter<uint32_t> reporter(spp, [&](int cur, int total) {
            fmt::print("Rendered : {}/{}spp Elapsed:{} Remaining:{}\n",
                       cur,
                       total, reporter.elapsedSeconds(), reporter.estimatedTimeToFinish());
            for (int i = 0; i < scene.filmDimension().x(); i++) {
                for (int j = 0; j < scene.filmDimension().y(); j++) {
                    scene.film->splatWeight({i, j}) = 1.0f / cur;
                }
            }
            scene.update();
        });
        std::vector<Seed> seeds(scene.filmDimension().x() * scene.filmDimension().y());
        {
            std::random_device rd;
            std::uniform_int_distribution<Seed> dist(1, UINT64_MAX);
            for (auto &i:seeds) {
                i = dist(rd);
            }
        }
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        for (int i = 0; i < spp && scene.processContinuable(); i++) {
            Thread::ParallelFor2D(scene.filmDimension(), [&](Point2i id, uint32_t threadId) {
                int idx = id.x() + id.y() * scene.filmDimension().x();
                auto temp = seeds[idx];
                SobolSampler sampler(&seeds[idx]);
                seeds[idx] = temp;
                sampler.startPass(i);
                auto ctx = scene.getRenderContext(id, &arenas[threadId], &sampler);
                auto Li = this->Li(ctx, scene);
                Li = clampRadiance(removeNaNs(Li), maxRayIntensity);
                scene.film->addSample(ctx.raster, Li, ctx.weight);
                arenas[threadId].reset();
            }, 4096);
            reporter.update();
        }
        for (int i = 0; i < scene.filmDimension().x(); i++) {
            for (int j = 0; j < scene.filmDimension().y(); j++) {
                scene.film->splatWeight({i, j}) = 1.0f / reporter.count();
            }
        }
        scene.update();
    }

}