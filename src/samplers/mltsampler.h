//
// Created by Shiina Miyuki on 2019/3/9.
//

#ifndef MIYUKI_MLTSAMPLER_H
#define MIYUKI_MLTSAMPLER_H

#include "sampler.h"

namespace Miyuki {
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

    class MLTSampler : public Sampler {
        int nStream;
        int streamIndex;
        int sampleIndex;
        std::vector<PrimarySample> X;
        int64_t currentIteration = 0, lastLargeStepIteration = 0;
        bool largeStep = true;
        Float largeStepProbability;

        void accept();

        void reject();

        int getNextIndex();

    public:
        MLTSampler(Seed *seed, int nStream) : Sampler(seed), nStream(nStream) {}

        void startStream(int index) {
            Assert(index < nStream);
            streamIndex = index;
            sampleIndex = 0;
        }

        void ensureReady(int index);

        Float get1D() override {
            return 0;
        }

        Point2f get2D() override {
            return Miyuki::Point2f();
        }

        void start() override;

        void startIteration();

    };
}
#endif //MIYUKI_MLTSAMPLER_H
