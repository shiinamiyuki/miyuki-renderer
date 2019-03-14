//
// Created by Shiina Miyuki on 2019/3/9.
//

#ifndef MIYUKI_MLTSAMPLER_H
#define MIYUKI_MLTSAMPLER_H

#include "sampler.h"

namespace Miyuki {
    // exponentially distributed between s1 and s2
    inline Float Mutate(Float random, Float value, Float S1, Float S2) {
        const float Factor = -std::log(S2 / S1);

        bool negative = random < 0.5f;
        random = negative ? random * 2.0f : (random - 0.5f) * 2.0f;

        float delta = S2 * std::exp(Factor * random);
        if (negative) {
            value -= delta;
            if (value < 0.0f)
                value += 1.0f;
        } else {
            value += delta;
            if (value >= 1.0f)
                value -= 1.0f;
        }
        if (value == 1.0f)
            value = 0.0f;
        return value;
    }

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
    protected:
        int nStream;
        int streamIndex;
        int sampleIndex;
        std::vector<PrimarySample> X;
        int64_t currentIteration = 0, lastLargeStepIteration = 0;
        bool largeStep = true;
        Float largeStepProbability;


        int getNextIndex();

    public:
        MLTSampler(Seed *seed, int nStream, Float largeStepProbability) : Sampler(seed), nStream(nStream),
                                                                          largeStepProbability(largeStepProbability) {}

        void startStream(int index) {
            Assert(index < nStream);
            streamIndex = index;
            sampleIndex = 0;
        }

        virtual void accept();

        virtual void reject();

        void mutate(PrimarySample &Xi, Float s1, Float s2);

        void ensureReady(int index);

        Float get1D() override {
            auto index = getNextIndex();
            ensureReady(index);
            Assert(X[index].value >= 0 && X[index].value <= 1);
            return X[index].value;
        }

        Point2f get2D() override {
            return {get1D(), get1D()};
        }

        void start() override;

        void startIteration();

    };
}
#endif //MIYUKI_MLTSAMPLER_H
