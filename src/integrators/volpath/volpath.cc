//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "volpath.h"
#include <utils/thread.h>
#include <core/scene.h>
#include <math/sampling.h>
#include <samplers/sobol.h>
#include <core/progress.h>
#include <boost/math/distributions/normal.hpp>
#include <thirdparty/hilbert/hilbert_curve.hpp>
#include <denoiser/denoiser.h>

namespace Miyuki {
    void VolPath::render(Scene &scene) {
        if (!adaptive) {
            if (!progressive) {
                renderNonProgressive(scene);
            } else {
                renderProgressive(scene);
            }
        } else {
            renderAdaptive(scene);
        }
    }

    void VolPath::renderAdaptive(Scene &scene) {
        fmt::print("Integrator: Volumetric Path Tracer (Adaptive)\nSamples per pixel:{}\n", spp);
        Float avgLuminance = 0;
        boost::math::normal normalDist(0.0, 1.0);
        double quantile = boost::math::quantile(normalDist, 1 - requiredPValue / 2);
        std::random_device rd;
        std::uniform_int_distribution<Seed> dist;
        {
            Seed seed = dist(rd);
            RandomSampler sampler(&seed);
            MemoryArena arena;
            for (int i = 0; i < nLuminanceSample; i++) {
                uint32_t x = sampler.uniformInt32() % scene.filmDimension().x();
                uint32_t y = sampler.uniformInt32() % scene.filmDimension().y();
                auto ctx = scene.getRenderContext(Point2i(x, y), &arena, &sampler);
                auto Li = this->Li(ctx, scene);
                auto luminance = Li.luminance();
                if (!std::isinf(luminance) && !std::isnan(luminance)) {
                    avgLuminance += luminance;
                }
                arena.reset();
            }
            avgLuminance /= nLuminanceSample;
            fmt::print("avgLuminance: {}, quantile: {}\n", avgLuminance, quantile);
        }
        /* Z = error / stdError
         * pValue = cdf(Z) * 2
         * pValue < required
         * cdf(Z) < required / 2
         * Z < cdf^(-1)(required / 2)
         * */
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

        ProgressReporter<uint32_t> reporter(hilbertMapping.size(), [&](uint32_t cur, uint32_t total) {
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
        {
            for (auto &i:seeds) {
                i = dist(rd);
            }
        }
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
                    SobolSampler sampler(&seeds[threadId]);
                    double mean = 0, meanSqr = 0.0f;
                    double sampleCount = 0;
                    while (scene.processContinuable()) {
                        // keeps minimum mem usage for cache efficiency
                        // should we do this?
                        arenas[threadId].reset();
                        auto ctx = scene.getRenderContext(raster, &arenas[threadId], &sampler);
                        auto Li = removeNaNs(this->Li(ctx, scene));
                        Li = clampRadiance(Li, maxRayIntensity);
                        auto sampleLuminance = Li.luminance();
                        sampleCount += 1;
                        double delta = sampleLuminance - mean;
                        mean += delta / sampleCount;
                        meanSqr += delta * (sampleLuminance - mean);

                        film.addSample(ctx.raster, Li, ctx.weight);
                        if (maxSampleFactor >= 0) {
                            if (sampleCount >= maxSampleFactor * spp) {
                                //fmt::print("break after max spp {}\n", sampleCount);
                                break;
                            }
                        }
                        if (sampleCount >= spp) {
                            auto variance = meanSqr / (sampleCount - 1);
                            auto stdError = std::sqrt(variance / sampleCount);
                            auto error = std::abs(mean - sampleLuminance) * heuristic;
                            //fmt::print("{} {}\n",error * maxError, stdError * quantile);
                            if (error * maxError >= stdError * quantile) {
                                //fmt::print("break after {} samples\n", sampleCount);
                                break;
                            }
                        }
                    }
                }
            }
            reporter.update();
        });
        scene.update();
    }

    Spectrum VolPath::Li(RenderContext &ctx, Scene &scene) {

        return LiWithAuxBuffer(ctx, scene).color.eval();
    }

    ShadingContext VolPath::LiWithAuxBuffer(RenderContext &ctx, Scene &scene) {
        struct VolPathTracer : PathTracer<VolPathTracer, ScatteringEvent> {
            VolPathTracer(bool s) {
                sampleDirect = s;
            }

            Float pdfImpl(ScatteringEvent &event) {
                return event.bsdf->pdf(event);
            }

            Spectrum sampleImpl(ScatteringEvent &event) {
                return event.bsdf->sample(event);
            }

            void makeScatteringEventImpl(ScatteringEvent *event, RenderContext &ctx, Intersection *intersection) {
                Integrator::makeScatteringEvent(event, ctx, intersection, TransportMode::radiance);
            }

            Spectrum nextEventEstimation(Scene &scene, RenderContext &ctx, const ScatteringEvent &event,
                                         Spectrum &sampledF,
                                         ScatteringEvent *sampledEvent,
                                         Intersection *intersection,
                                         bool *sampleValid) {
                const auto &lights = scene.lights;
                if (lights.empty())
                    return {};
                auto lightDistribution = scene.lightDistribution.get();
                return sampleOneLightMIS(scene, lights, *lightDistribution,
                                         ctx, event, sampledF, sampledEvent, intersection, sampleValid);
            }
        };
        return VolPathTracer(sampleDirect).Li(scene, ctx.primary, ctx, minDepth, maxDepth);
    }


    VolPath::VolPath(const ParameterSet &set) {
        progressive = set.findInt("integrator.progressive", false);
        minDepth = set.findInt("integrator.minDepth", 1);
        maxDepth = set.findInt("integrator.maxDepth", 5);
        spp = set.findInt("integrator.spp", 4);
        maxRayIntensity = set.findFloat("integrator.maxRayIntensity", 10000.0f);
        caustics = set.findInt("integrator.caustics", true);
        adaptive = set.findInt("integrator.adaptive", false);
        nLuminanceSample = set.findInt("integrator.nLuminanceSample", 100000);
        maxError = set.findFloat("integrator.maxError", 0.05);
        requiredPValue = set.findFloat("integrator.pValue", 0.05);
        maxSampleFactor = set.findFloat("integrator.maxSampleFactor", 32);
        heuristic = set.findFloat("integrator.heuristic", 0.25);
    }

    void VolPath::renderProgressive(Scene &scene) {
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

    void VolPath::renderNonProgressive(Scene &scene) {
        fmt::print("Integrator: Volumetric Path Tracer\nSamples per pixel:{}\n", spp);
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
        film.enableAuxBuffer();
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        Thread::ParallelFor(0u, hilbertMapping.size(), [&](uint32_t idx, uint32_t threadId) {
            int tx, ty;
            tx = hilbertMapping[idx].x();
            ty = hilbertMapping[idx].y();
            Point2i tilePos(tx, ty);
            tilePos *= TileSize;
            Bound2i tileBound(tilePos, tilePos + Point2i{TileSize, TileSize});
//            auto tile = film.getFilmTile(tileBound);
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
                        auto shading = this->LiWithAuxBuffer(ctx, scene);
                        auto Li = removeNaNs(shading.color.eval());
                        Li = clampRadiance(Li, maxRayIntensity);
                        film.addSample(ctx.raster, Li, ctx.weight);
                        film.addSample(ctx.raster, shading);
                    }
                }
            }
            //  film.mergeFilmTile(*tile);
            reporter.update();

        });
        scene.update();
        fmt::print("Denoising\n");
        Denoiser denoiser;
        denoiser.denoise(film);
        scene.update();
    }


}