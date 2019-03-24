//
// Created by Shiina Miyuki on 2019/3/22.
//

#ifndef MIYUKI_PSSMLT_H
#define MIYUKI_PSSMLT_H

#include <integrators/volpath/volpath.h>
#include <samplers/mltsampler.h>
#include <integrators/mmlt/mmlt.h>

namespace Miyuki {
    // Unidirectional PSSMLT only makes things worse
    class PSSMLT : public VolPath {
    protected:
        int nBootstrap;
        int nChains;
        int64_t nMutations;
        Float b;
        Float largeStep;
        int nDirect;
        std::vector<Seed> mltSeeds;
        DECLARE_STATS_MEMBER(uint64_t, acceptanceCounter);
        DECLARE_STATS_MEMBER(uint64_t, mutationCounter);
        std::vector<std::shared_ptr<MLTSampler>> samplers;
        std::unique_ptr<ProgressReporter<uint64_t>> reporter;
        std::mutex mutex;

        Spectrum radiance(Scene &scene, MemoryArena *arena, MLTSampler *sampler, Point2i *raster);

        void handleDirect(Scene &scene);

    public:
        PSSMLT(const ParameterSet &set) : VolPath(set) {
            nBootstrap = set.findInt("integrator.nBootstrap", 100000);
            nDirect = set.findInt("integrator.nDirect", 16);
            nChains = set.findInt("integrator.nChains", 1000);
            largeStep = set.findFloat("integrator.largeStep", 0.25f);
            b = 0;
        }

        void render(Scene &scene) override;


        void renderProgressive(Scene &scene);

        void renderNonProgressive(Scene &scene);

        void recoverImage(Scene &scene);

    private:

        void generateBootstrapSamples(Scene &scene);

        void runMC(Scene &scene, MLTSampler *sampler, MemoryArena *arena);
    };
}
#endif //MIYUKI_PSSMLT_H
