//
// Created by Shiina Miyuki on 2019/4/5.
//

#include "vpl.h"
#include <bidir/vertex.h>
#include <utils/thread.h>

namespace Miyuki {
    void VPL::render(Miyuki::Scene &scene) {
        auto film = scene.getFilm();
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        std::vector<Seed> seeds(Thread::pool->numThreads());
        {
            std::random_device rd;
            std::uniform_int_distribution<Seed> dist(1, UINT64_MAX);
            for (auto &i:seeds) {
                i = dist(rd);
            }
        }
        MemoryArena a1;
        while (scene.processContinuable()) {
            Bidir::SubPath path(nullptr, 0);
            {
                a1.reset();
                RandomSampler sampler(&seeds[0]);
                auto ctx = scene.getRenderContext(Point2i{0, 0}, &a1, &sampler);
                path = Bidir::GenerateLightSubPath(scene, ctx, minDepth, maxDepth);
            }
            Thread::ParallelFor2D(scene.filmDimension(), [&](Point2i id, uint32_t threadId) {
                RandomSampler sampler(&seeds[threadId]);
                auto ctx = scene.getRenderContext(id, &arenas[threadId], &sampler);
                auto eyePath = Bidir::GenerateCameraSubPath(scene, ctx, minDepth, maxDepth);
                Spectrum Li;
                if (eyePath.N > 1) {
                    Li += eyePath[1].Le(eyePath[0]);
                    for (int i = 1; i < path.N; i++) {
                        Point2f _;
                        Li += Bidir::ConnectBDPT(scene, ctx, path, eyePath, i, 2, &_, false, nullptr);
                    }
                }
                film->addSample(ctx.raster, Li, ctx.weight);
                arenas[threadId].reset();
            }, 4096);
            scene.update();
        }
    }

    VPL::VPL(const ParameterSet &set) {
        minDepth = set.findInt("integrator.minDepth", 3);
        maxDepth = set.findInt("integrator.maxDepth", 5);
    }
}
