//
// Created by Shiina Miyuki on 2019/3/13.
//

#ifndef MIYUKI_MMLT_H
#define MIYUKI_MMLT_H

#include <samplers/mltsampler.h>
#include <integrators/integrator.h>
#include <integrators/bdpt/bdpt.h>
#include <utils/stats.hpp>
#include <core/progress.h>
#include <integrators/direct.hpp>

namespace Miyuki {
    inline double AverageMutationPerPixel(int64_t nPixels, int64_t nChains, int64_t nIterations) {
        return nChains * nIterations / (double) nPixels;
    }

    inline int64_t ChainsMutations(int64_t nPixels, int64_t nChains, double mpp) {
        return std::round(mpp * nPixels / nChains);
    }

    struct MLTStats {
        using StatsVec = std::vector<std::shared_ptr<std::atomic<uint64_t>>>;
        StatsVec mutations;
        StatsVec accepts;
        StatsVec rejects;

        MLTStats(int maxLength){
            for(int i =0 ;i<maxLength;i++){
                mutations[i] = std::make_shared<std::atomic<uint64_t>>(0);
                accepts[i] = std::make_shared<std::atomic<uint64_t>>(0);
                rejects[i] = std::make_shared<std::atomic<uint64_t>>(0);
            }
        }
        void mutate(int pathLength) {
            (*mutations[pathLength])++;
        }

        void accept(int pathLength) {
            (*accepts[pathLength])++;
        }

        void reject(int pathLength) {
            (*rejects[pathLength])++;
        }
    };

    struct MarkovChain {
        // seeds.size() = sampler.size() = maxDepth + 1
        std::vector<Seed> seeds;
        std::vector<std::shared_ptr<MLTSampler>> samplers;
        Distribution1D pathLengthDistribution;

        MarkovChain(const std::vector<Float> &b) :
                seeds(b.size() + 1u), samplers(b.size() + 1u),
                pathLengthDistribution(&b[0], b.size()) {}

        MarkovChain &operator=(const MarkovChain &) = delete;

    };

    class MultiplexedMLT : public BDPT {
        int nBootstrap;
        int nChains;
        int64_t nMutations;
        Float largeStep;
        int nDirect;
        bool useKelemenWeight;
        static const int cameraStreamIndex;
        static const int lightStreamIndex;
        static const int connectionStreamIndex;
        static const int nStream;

        static const int twoStageSampleFactor;
        DECLARE_STATS_MEMBER(uint64_t, acceptanceCounter);
        DECLARE_STATS_MEMBER(uint64_t, mutationCounter);
        std::unique_ptr<ProgressReporter<uint64_t>> reporter;
        std::mutex mutex;
    protected:
        std::vector<Float> b;
        std::vector<std::shared_ptr<MarkovChain>> chains;

        Spectrum radiance(Scene &scene, MemoryArena *arena, MLTSampler *sampler, int depth, Point2f *raster);

        void handleDirect(Scene &scene);

        void generateBootstrapSamples(Scene &scene);

        void run(MarkovChain & markovChain,Scene &scene, MemoryArena *arena);


        void recoverImage(Scene &scene);


    public:
        void render(Scene &scene) override;

        MultiplexedMLT(const ParameterSet &set);
    };
}
#endif //MIYUKI_MMLT_H
