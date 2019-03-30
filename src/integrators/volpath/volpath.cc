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

namespace Miyuki {
    void VolPath::render(Scene &scene) {
        if (!adaptive) {
            fmt::print("Integrator: Volumetric Path Tracer\nSamples per pixel:{}\n", spp);
            SamplerIntegrator::render(scene);
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

                        film.addSample({x, y}, Li, ctx.weight);
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

//    Spectrum VolPath::Li(RenderContext &ctx, Scene &scene) {
//        RayDifferential ray = ctx.primary;
//        Intersection intersection;
//        ScatteringEvent event;
//        Spectrum Li(0, 0, 0);
//        Spectrum beta(1, 1, 1);
//        bool specular = false;
//        for (int depth = 0; depth < maxDepth; depth++) {
//            if (!scene.intersect(ray, &intersection)) {
//                Li += beta * scene.infiniteAreaLight->L(ray);
//                break;
//            }
//            if (depth > 0 && !sampleIndirect) {
//                break;
//            }
//            makeScatteringEvent(&event, ctx, &intersection, TransportMode::radiance);
//            if ((caustics && specular) || depth == 0) {
//                Li += event.Le(-1 * ray.d) * beta;
//            }
//            if(depth > 0 || sampleDirect)
//                Li += beta * estimateDirect(scene, ctx, event);
//            auto f = event.bsdf->sample(event);
//            specular = event.bsdfLobe.matchFlag(BSDFLobe::specular);
//            if (event.pdf <= 0 || f.isBlack()) {
//                break;
//            }
//            ray = event.spawnRay(event.wiW);
//            beta *= f * Vec3f::absDot(event.wiW, event.Ns()) / event.pdf;
//            Float p = beta.max();
//            if (depth >= minDepth && p < 1) {
//                if (ctx.sampler->get1D() < p) {
//                    beta /= p;
//                } else {
//                    break;
//                }
//            }
//        }
//        return Li;
//    }


    Spectrum VolPath::Li(RenderContext &ctx, Scene &scene) {
        struct VolPathTracer : PathTracer<VolPathTracer> {
            Float pdfEvent(ScatteringEvent &event) {
                return event.bsdf->pdf(event);
            }

            Spectrum sampleEvent(ScatteringEvent &event) {
                return event.bsdf->sample(event);
            }

            Spectrum nextEventEstimation(Scene &scene, RenderContext &ctx, const ScatteringEvent &event) {
                const auto &lights = scene.lights;
                if (lights.empty())
                    return {};
                auto lightDistribution = scene.lightDistribution.get();
                return sampleOneLightMIS(scene, lights, *lightDistribution, ctx, event);
            }
        };
        return VolPathTracer().Li(scene, ctx.primary, ctx, minDepth, maxDepth);
    }
    

    VolPath::VolPath(const ParameterSet &set) {
        progressive = set.findInt("integrator.progressive", false);
        minDepth = set.findInt("integrator.minDepth", 3);
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
        SamplerIntegrator::renderProgressive(scene);
    }


}