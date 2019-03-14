//
// Created by Shiina Miyuki on 2019/3/13.
//

#ifndef MIYUKI_MMLT_H
#define MIYUKI_MMLT_H

#include <samplers/mltsampler.h>
#include <integrators/integrator.h>
#include <integrators/bdpt/bdpt.h>

namespace Miyuki {
    class MMLTSampler : public MLTSampler {
        PrimarySample u1, u2;// for image location
        void ensureReadyU1U2();

    public:
        Point2i imageLocation;
        Spectrum L;
        Point2i imageDimension;
        int depth;

        void accept() override;

        void reject() override;

        Point2i sampleImageLocation() {
            ensureReadyU1U2();
            int x = clamp<int>(lroundf(u1.value * imageDimension.x()), 0, imageDimension.x() - 1);
            int y = clamp<int>(lroundf(u2.value * imageDimension.y()), 0, imageDimension.y() - 1);
            return {x, y};
        }

        MMLTSampler(Seed *seed, int nStream, Float largeStep, Point2i imageDimension, int depth)
                : MLTSampler(seed, nStream, largeStep), imageDimension(imageDimension), depth(depth) {}
    };

    inline Float AverageMutationPerPixel(int nPixels, int nChains, int nIterations) {
        return nChains * nIterations / (Float) nPixels;
    }

    inline int ChainsMutations(int nPixels, int nChains, int mpp) {
        return std::round(mpp * nPixels / nChains);
    }

    class MultiplexedMLT : public BDPT {
        int nBootstrap;
        int nChains;
        int nMutations;
        Float b;
        Float largeStep;
        int nDirect;
        static const int cameraStreamIndex = 0;
        static const int lightStreamIndex = 1;
        static const int connectionStreamIndex = 2;
        static const int nStream = 3;
        int curIter = 0;
    protected:

        Spectrum radiance(Scene &scene, MemoryArena *arena, MMLTSampler *sampler, int depth, Point2i *raster);

        void handleDirect(Scene &scene);

    public:
        void render(Scene &scene) override;

        MultiplexedMLT(const ParameterSet &set);
    };
}
#endif //MIYUKI_MMLT_H
