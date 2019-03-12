//
// Created by Shiina Miyuki on 2019/3/11.
//

#include "metropolis.h"
#include <utils/thread.h>

namespace Miyuki {

    void
    MetropolisBootstrapper::generateBootstrapSamples(BootstrapSample *samples, uint32_t nBootstrap, uint32_t nChains) {
        std::random_device rd;
        // compute bootstrap samples
        std::vector<Seed> seeds(nBootstrap);
        std::vector<Seed> bootstrapSeeds(nBootstrap);
        std::vector<Float> bootstrapWeights(nBootstrap);
        {
            std::uniform_int_distribution<Seed> dist;
            for (int i = 0; i < seeds.size(); i++) {
                seeds[i] = dist(rd);
                bootstrapSeeds[i] = seeds[i];

            }
        }
        std::vector<MemoryArena> arenas(Thread::pool->numThreads());
        Thread::ParallelFor(0u, nBootstrap, [&](uint32_t idx, uint32_t threadId) {
            bootstrapWeights[idx] = f(&bootstrapSeeds[idx],&arenas[threadId]);
        });
        Distribution1D bootstrap(bootstrapWeights.data(), nBootstrap);
        samples->b = bootstrap.funcInt / nBootstrap;
        samples->seeds.resize(nChains);
        std::uniform_real_distribution<Float> dist;
        for (int i = 0; i < nChains; i++) {
            auto seedIndex = bootstrap.sampleInt(dist(rd));
            samples->seeds[i] = seeds[seedIndex];
        }
    }
}