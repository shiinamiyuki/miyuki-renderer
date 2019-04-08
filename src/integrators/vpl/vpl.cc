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
            Light *light = nullptr;
            Float pdf;
            Point2f u;
            {
                a1.reset();
                RandomSampler sampler(&seeds[0]);
                auto ctx = scene.getRenderContext(Point2i{0, 0}, &a1, &sampler);
                path = Bidir::GenerateLightSubPath(scene, ctx, minDepth, maxDepth);
                light = scene.chooseOneLight(&sampler, &pdf);
                if (!light)break;
                u = sampler.get2D();
            }

            Thread::ParallelFor2D(scene.filmDimension(), [&](Point2i id, uint32_t threadId) {
                RandomSampler sampler(&seeds[threadId]);
                auto ctx = scene.getRenderContext(id, &arenas[threadId], &sampler);
                auto eyePath = Bidir::GenerateCameraSubPath(scene, ctx, minDepth, maxDepth);
                Spectrum Li;
                if (eyePath.N > 1) {
                    Vec3f wi;
                    VisibilityTester tester;
                    Float p;
                    if (eyePath[1].event) {
                        auto L = light->sampleLi(u, *eyePath[1].event->getIntersection(), &wi, &p, &tester);
                        if (!L.isBlack() && p > 0 && eyePath[1].event) {
                            auto e = *eyePath[1].event;
                            e.wi = e.worldToLocal(wi);
                            e.wiW = wi;
                            Spectrum Ld = L * e.bsdf->f(e) * Vec3f::absDot(e.Ns(), wi) / p / pdf;
                            if (!Ld.isBlack() && tester.visible(scene)) {
                                Li += Ld;
                            }
                        }
                    }
                    Li += eyePath[1].Le(eyePath[0]);
                    for (int i = 2; i < path.N; i++) {
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
