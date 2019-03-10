//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "volpath.h"
#include "utils/thread.h"
#include "core/scene.h"
#include "math/sampling.h"
#include "core/progress.h"
#include "thirdparty/hilbert/hilbert_curve.hpp"
#include "samplers/sobol.h"

namespace Miyuki {
    void VolPath::render(Scene &scene) {
        fmt::print("Integrator: Volumetric Path Tracer\nSamples per pixel:{}\n", spp);
        auto &film = *scene.film;
        Point2i nTiles = film.imageDimension() / TileSize + Point2i{1, 1};

        // 2^M >= nTiles.x() * nTiles.y()
        // M >= log2(nTiles.x() * nTiles.y());
        int M = std::ceil(std::log2(nTiles.x() * nTiles.y()));
        Point2i nTiles2 = Point2i(std::pow(2, M / 2), std::pow(2, M / 2));
        std::mutex mutex;
        std::vector<Point2f> hilbertMapping;
        for (int i = 0; i < nTiles2.x() * nTiles2.y(); i++) {
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
            arenas[threadId].reset();
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
                    for (int s = 0; s < spp; s++) {
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

    Spectrum VolPath::L(RenderContext &ctx, Scene &scene) {
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
            if (event.pdf < 0) {
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

    VolPath::VolPath(const ParameterSet &set) {
        progressive = false;
        minDepth = set.findInt("volpath.minDepth", 3);
        maxDepth = set.findInt("volpath.maxDepth", 5);
        spp = set.findInt("volpath.spp", 4);
        maxRayIntensity = set.findFloat("volpath.maxRayIntensity", 10000.0f);
        caustics = set.findInt("volpath.caustics", true);
    }
}