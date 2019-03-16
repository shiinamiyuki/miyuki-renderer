//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "volpath.h"
#include <utils/thread.h>
#include <core/scene.h>
#include <math/sampling.h>
#include <bidir/vertex.h>
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
                    auto Li = L(ctx, scene);
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
                std::random_device rd;
                std::uniform_int_distribution<Seed> dist;
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
                            auto Li = removeNaNs(L(ctx, scene));
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
                                auto error = std::max<double>(mean, avgLuminance * heuristic);
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
    }

    // TODO: try maxDepth=1
    Spectrum VolPath::LPathTraced(RenderContext &ctx, Scene &scene) {
        RayDifferential ray = ctx.primary;
        Intersection intersection;
        ScatteringEvent event;
        Spectrum Li(0, 0, 0);
        Spectrum beta(1, 1, 1);
        bool specular = false;
        for (int depth = 0; depth < maxDepth; depth++) {
            if (!scene.intersect(ray, &intersection)) {
                break;
            }
            makeScatteringEvent(&event, ctx, &intersection);
            if ((caustics && specular) || depth == 0) {
                Li += event.Le(-1 * ray.d) * beta;
            }
            Li += beta * importanceSampleOneLight(scene, ctx, event);
            auto f = event.bsdf->sample(event);
            specular = event.bsdfLobe.matchFlag(BSDFLobe::specular);
            if (event.pdf <= 0) {
                break;
            }
            ray = event.spawnRay(event.wiW);
            beta *= f * Vec3f::absDot(event.wiW, event.Ns()) / event.pdf;
            if (depth >= minDepth) {
                Float p = beta.max();
                if (ctx.sampler->get1D() < p) {
                    beta /= p;
                } else {
                    break;
                }
            }
        }
        return Li;
    }

    Spectrum VolPath::LRandomWalk(RenderContext &ctx, Scene &scene) {
        using Bidir::Vertex, Bidir::SubPath;
        auto vertices = ctx.arena->alloc<Bidir::Vertex>(size_t(maxDepth + 1));
        Spectrum beta(1, 1, 1);
        vertices[0] = Bidir::CreateCameraVertex(ctx.camera, ctx.raster, ctx.primary, 1.0f, beta);
        auto path = Bidir::RandomWalk(vertices + 1, ctx.primary, beta,
                                      1.0f, scene, ctx, minDepth, maxDepth,
                                      Bidir::TransportMode::importance);
        Spectrum Li(0, 0, 0);
        bool specular = false;
        ctx.sampler->startDimension(4 + 3 * maxDepth);
        for (int depth = 0; depth < path.N; depth++) {
            if (specular || depth == 0) {
                Vec3f wo = (path[depth - 1].ref - path[depth].ref).normalized();
                Li += path[depth].beta * path[depth].Le(wo);
            }
            Li += path[depth].beta * importanceSampleOneLight(scene, ctx, *path[depth].event);
            specular = path[depth].delta;
        }
        return Li;
    }


    VolPath::VolPath(const ParameterSet &set) {
        progressive = false;
        minDepth = set.findInt("volpath.minDepth", 3);
        maxDepth = set.findInt("volpath.maxDepth", 5);
        spp = set.findInt("volpath.spp", 4);
        maxRayIntensity = set.findFloat("volpath.maxRayIntensity", 10000.0f);
        caustics = set.findInt("volpath.caustics", true);
        adaptive = set.findInt("volpath.adaptive", false);
        nLuminanceSample = set.findInt("volpath.nLuminanceSample", 100000);
        maxError = set.findFloat("volpath.maxError", 0.05);
        requiredPValue = set.findFloat("volpath.pValue", 0.05);
        maxSampleFactor = set.findFloat("volpath.maxSampleFactor", 32);
        heuristic = set.findFloat("volpath.heuristic", 0.1);
    }
}