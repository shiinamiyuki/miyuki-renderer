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

    class MultiplexedMLT : public BDPT {
        int nBootstrap;
        int nChains;
        int64_t nMutations;
        Float b;
        Float largeStep;
        int nDirect;
        static const int cameraStreamIndex;
        static const int lightStreamIndex;
        static const int connectionStreamIndex;
        static const int nStream;

        static const int twoStageSampleFactor;
        DECLARE_STATS_MEMBER(uint64_t, acceptanceCounter);
        DECLARE_STATS_MEMBER(uint64_t, mutationCounter);
        std::unique_ptr<ProgressReporter<uint64_t>> reporter;
        std::mutex mutex;
        bool twoStage;
        Point2i twoStageResolution;
        std::vector<AtomicFloat> twoStageTestImage;
    protected:

        std::vector<Seed> mltSeeds;
        std::vector<std::shared_ptr<MLTSampler>> samplers;

        Spectrum radiance(Scene &scene, MemoryArena *arena, MLTSampler *sampler, int depth, Point2i *raster);

        void handleDirect(Scene &scene);

        void twoStageInit(Scene &scene);

        void generateBootstrapSamples(Scene &scene);

        void runMC(Scene &scene, MLTSampler *sampler, MemoryArena *arena);

        // MLT is after all a progressive algorithm
        // The only difference is the order of MC being executed
        void renderProgressive(Scene &scene);

        void renderNonProgressive(Scene &scene);

        void recoverImage(Scene &scene);

        Float approxLuminance(const Point2i &raster) {
            int x = raster.x() / twoStageSampleFactor;
            int y = raster.y() / twoStageSampleFactor;
            Float lum = twoStageTestImage[x + twoStageResolution.x() * y];
            lum = std::min(lum, 10.0f);
            if (lum <= 0 || std::isnan(lum))
                return 0.01;
            return lum;
        }

    public:
        void render(Scene &scene) override;

        MultiplexedMLT(const ParameterSet &set);
    };
}
#endif //MIYUKI_MMLT_H
