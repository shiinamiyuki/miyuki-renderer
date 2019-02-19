//
// Created by Shiina Miyuki on 2019/2/18.
//

#ifndef MIYUKI_MMLT_H
#define MIYUKI_MMLT_H

#include "../integrator.h"
#include "../../core/spectrum.h"
#include "../../samplers/random.h"
#include "../../core/memory.h"
#include "../bdpt/bdpt.h"

namespace Miyuki {
    class MLTSampler : public RandomSampler {
    protected:
        struct PrimarySample {
            Float value, valueBackup;
            int64_t lastModificationIteration;
            int64_t modifyBackup;

            explicit PrimarySample(Float v = 0) : value(v), valueBackup(0), lastModificationIteration(0),
                                                  modifyBackup(0) {}

            void backup() {
                valueBackup = value;
                modifyBackup = lastModificationIteration;
            }

            void restore() {
                lastModificationIteration = modifyBackup;
                value = valueBackup;
            }
        };

        void ensureReady(int index);

    private:
        Float sigma, largeStepProbability;
        std::vector<PrimarySample> X;
        int64_t currentIteration, lastLargeStepIteration;
        bool largeStep;
        int streamIndex, sampleIndex, streamCount;

        void accept();

        void reject();

        int getNextIndex();

    public:
        MLTSampler(Seed *seed) : RandomSampler(seed),
                                 currentIteration(0), largeStep(true), lastLargeStepIteration(0) {}

        Float nextFloat() override;

        Point2f nextFloat2D() override;

        void start() override;

        void startIteration();

        void startStream(int index);

    };

    class MultiplexedMLT : public BDPT {
        ConcurrentMemoryArenaAllocator arenaAllocator;
    public:
        Spectrum L(Scene &scene, MemoryArena &arena, MLTSampler &sampler, int depth, Point2i *raster);

        void iteration(Scene &scene) override;

        void render(Scene &scene) override;

    };
}
#endif //MIYUKI_MMLT_H
