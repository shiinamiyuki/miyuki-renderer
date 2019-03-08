//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "volpath.h"
#include "utils/thread.h"
#include "core/scene.h"
#include "math/sampling.h"
#include "core/progress.h"

namespace Miyuki {
    void VolPath::render(Scene &scene) {
        fmt::print("Integrator: Volumetric Path Tracer\nSamples per pixel:{}\n", spp);
        auto &film = *scene.film;
        Point2i nTiles = film.imageDimension() / TileSize + Point2i{1, 1};
        ProgressReporter reporter(nTiles.x() * nTiles.y(), [&](int cur, int total) {
            if (cur % (total / 100) == 0)
                fmt::print("Rendered tiles: {}/{} Elapsed:{} Remaining:{}\n",
                           cur,
                           total, reporter.elapsedSeconds(), reporter.estimatedTimeToFinish());
        });
        Thread::parallelFor2D(nTiles, [&](Point2i tile, uint32_t threadId) {
            scene.arenas[threadId].reset();
            for (int i = 0; i < TileSize; i++) {
                for (int j = 0; j < TileSize; j++) {
                    int x = tile.x() * TileSize + i;
                    int y = tile.y() * TileSize + j;
                    if (x >= film.width() || y >= film.height())
                        continue;
                    for (int s = 0; s < spp; s++) {
                        auto raster = Point2i{x, y};
                        auto ctx = scene.getRenderContext(raster, &scene.arenas[threadId]);
                        auto Li = L(ctx, scene);
                        film.addSample({x, y}, Li, ctx.weight);
                    }
                }
            }
            reporter.update();
        });
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
            if (specular || depth == 0) {
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
        progressive = true;
        minDepth = set.findInt("volpath.minDepth", 3);
        maxDepth = set.findInt("volpath.maxDepth", 5);
        spp = set.findInt("volpath.spp", 4);
    }
}