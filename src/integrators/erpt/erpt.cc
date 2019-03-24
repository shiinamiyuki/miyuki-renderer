//
// Created by Shiina Miyuki on 2019/3/22.
//

#include <utils/thread.h>
#include "erpt.h"

namespace Miyuki {

    void ERPT::render(Scene &scene) {
        fmt::print("Integrator: Energy Redistribution Path Tracing on PSSMLT\n");
        handleDirect(scene);
        estimateDepositionEnergy(scene);
        SamplerIntegrator::render(scene);
    }

    ERPT::ERPT(const ParameterSet &set) : PSSMLT(set) {
        nMutations = set.findInt("integrator.nMutations", 100);
    }

    void ERPT::estimateDepositionEnergy(Scene &scene) {
        fmt::print("Estimating Deposition Energy\n");
        e_d = 0;
        std::random_device rd;
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        std::vector<Seed> seeds(Thread::pool->numThreads());
        std::uniform_int_distribution<unsigned short> dist;
        for (int i = 0; i < seeds.size(); i++) {
            seeds[i] = dist(rd);
        }
        Thread::ParallelFor(0u, nBootstrap, [&](uint32_t i, uint32_t threadId) {
            arenas[threadId].reset();
            Point2i raster;
            MLTSampler sampler(&seeds[threadId], 1, largeStep, scene.filmDimension(), maxDepth);
            auto L = radiance(scene, &arenas[threadId], &sampler,
                              &raster);
            L = clampRadiance(removeNaNs(L), maxRayIntensity);
            e_d += L.luminance();
        }, 4096);
        e_d = e_d / nBootstrap;
        fmt::print("e_d = {}\n", e_d);
    }

    Spectrum ERPT::Li(RenderContext &ctx, Scene &scene) {
        MLTSampler sampler(ctx.sampler->getSeed(), 1, largeStep, scene.filmDimension(), maxDepth);
        sampler.maxImagePlaneStratification = 16.0f / (scene.filmDimension().x() + scene.filmDimension().y());
        auto L0 = MCMCFirstIteration(ctx.raster, scene, ctx.arena, &sampler);
        auto e = L0.luminance();
        if (e <= 0)
            return {};
        
        int numChains = std::floor(sampler.uniformFloat() + e / (spp * e_d * nMutations));

        Float dep = e / (spp * numChains);
        for (int i = 0; i < numChains; i++) {
            MLTSampler chain = sampler;
            for (int j = 0; j < nMutations; j++) {
                chain.startIteration();
                Point2i pProposed;
                auto LProposed = radiance(scene, ctx.arena, &chain, &pProposed);
                Float q;
                if (chain.L.luminance() == 0 && LProposed.luminance() > 0)
                    q = 1;
                else
                    q = clamp(LProposed.luminance() / chain.L.luminance(), 0.0f, 1.0f);
                if (q == 1 || ctx.sampler->uniformFloat() < q) {
                    chain.L = LProposed;
                    chain.imageLocation = pProposed;
                    chain.accept();

                } else {
                    chain.reject();
                }
                scene.film->addSplat(chain.imageLocation, LProposed / e * dep);
            }
        }
        return {};
    }

    Spectrum ERPT::MCMCFirstIteration(const Point2i &raster, Scene &scene, MemoryArena *arena, MLTSampler *sampler) {
        sampler->sampleImageLocation();
        Point2f u(raster.x(), raster.y());
        u /= Point2f(scene.filmDimension().x(), scene.filmDimension().y());
        sampler->u1.value = u[0];
        sampler->u1.valueBackup = u[0];
        sampler->u2.value = u[1];
        sampler->u2.valueBackup = u[1];
        sampler->startStream(0);
        auto ctx = scene.getRenderContext(raster, arena, sampler);
        auto L = VolPath::Li(ctx, scene);
        return L;
    }
}