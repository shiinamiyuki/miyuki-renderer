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
    class MMLTSampler : public MLTSampler {
        PrimarySample u1, u2;// for image location
        void ensureReadyU1U2();

    public:
        Point2i imageLocation;
        Spectrum L;
        Point2i imageDimension;
        int depth;
        int rejectCount = 0;
        uint64_t smallAcceptCount = 0;
        uint64_t largeAcceptCount = 0;
        uint64_t smallCount = 0;
        uint64_t largeCount = 0;
        uint64_t nonZeroCount = 0;

        Point2i sampleImageLocation() {
            ensureReadyU1U2();
            int x = clamp<int>(lroundf(u1.value * imageDimension.x()), 0, imageDimension.x() - 1);
            int y = clamp<int>(lroundf(u2.value * imageDimension.y()), 0, imageDimension.y() - 1);
            return {x, y};
        }

        MMLTSampler(Seed *seed, int nStream, Float largeStep, Point2i imageDimension, int depth)
                : MLTSampler(seed, nStream, largeStep),
                  imageDimension(imageDimension), depth(depth) {}

        bool large() const {
            return largeStep;
        }

        void setLarge(Float large) {
            largeStepProbability = large;
        }

        void accept() override {
            rejectCount = 0;
            if (large()) {
                largeAcceptCount++;
            } else {
                smallAcceptCount++;
            }
            MLTSampler::accept();
        }

        double ns() const {
            return (double) smallAcceptCount / smallCount;
        }

        double nl() const {
            return (double) largeAcceptCount / largeCount;
        }

        double n0() const {
            return (double) nonZeroCount / largeCount;
        }

        double pl() const {
            auto n = nl() / n0();
            if (n < 0.1 || n0() == 0) {
                return 0.25;
            }
            return std::max(0.01, ns() / (2 * (ns() - nl())));
        }

        void startIteration() override {
//            if (smallCount + largeCount == 4096) {
//                largeStepProbability = (Float) pl();
//            }
            MLTSampler::startIteration();
            if (largeStep) {
                largeCount++;
            } else {
                smallCount++;
            }

        }

        void reject() override {
            if (u1.lastModificationIteration == currentIteration)
                u1.restore();
            if (u2.lastModificationIteration == currentIteration)
                u2.restore();
            MLTSampler::reject();
            rejectCount++;
        }
    };

    inline double AverageMutationPerPixel(int64_t nPixels, int64_t nChains, int64_t nIterations) {
        return nChains * nIterations / (double) nPixels;
    }

    inline int64_t ChainsMutations(int64_t nPixels, int64_t nChains, double mpp) {
        return std::round(mpp * nPixels / nChains);
    }

    class MultiplexedMLT : public BDPT {
        int nBootstrap;
        int nChains;
        int64_t nMutations;
        Float b;
        Float largeStep;
        int nDirect;
        bool progressive;
        static const int cameraStreamIndex = 0;
        static const int lightStreamIndex = 1;
        static const int connectionStreamIndex = 2;
        static const int nStream = 3;
        int maxConsecutiveRejects;
        static const int twoStageSampleFactor = 50;
        DECLARE_STATS_MEMBER(int32_t, acceptanceCounter);
        DECLARE_STATS_MEMBER(int32_t, mutationCounter);
        std::unique_ptr<ProgressReporter<uint64_t>> reporter;
        std::mutex mutex;
        bool twoStage;
        Point2i twoStageResolution;
        std::vector<AtomicFloat> twoStageTestImage;
    protected:

        std::vector<Seed> mltSeeds;
        std::vector<std::shared_ptr<MMLTSampler>> samplers;

        Spectrum radiance(Scene &scene, MemoryArena *arena, MMLTSampler *sampler, int depth, Point2i *raster);

        void handleDirect(Scene &scene);

        void twoStageInit(Scene &scene);

        void generateBootstrapSamples(Scene &scene);

        void runMC(Scene &scene, MMLTSampler *sampler, MemoryArena *arena);

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
