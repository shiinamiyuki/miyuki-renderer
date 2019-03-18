//
// Created by Shiina Miyuki on 2019/3/9.
//

#ifndef MIYUKI_MLTSAMPLER_H
#define MIYUKI_MLTSAMPLER_H

#include "sampler.h"
#include <math/func.h>

#define USE_KELEMEN_MUTATION 1

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

        inline void mutate(PrimarySample &Xi, Float s1, Float s2);

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

        virtual void startIteration();

    };

    inline void MLTSampler::mutate(PrimarySample &Xi, Float s1, Float s2) {
        if (Xi.lastModificationIteration < lastLargeStepIteration) {
            Xi.value = uniformFloat();
            Xi.lastModificationIteration = lastLargeStepIteration;
        }

        if (largeStep) {
            Xi.backup();
            Xi.value = uniformFloat();
        } else {
            int64_t nSmall = currentIteration - Xi.lastModificationIteration;
#if USE_KELEMEN_MUTATION == 1
            auto nSmallMinus = nSmall - 1;
            if (nSmallMinus > 0) {
                auto x = Xi.value;
                while (nSmallMinus > 0) {
                    nSmallMinus--;
                    x = Mutate(uniformFloat(), x, s1, s2);
                }
                Xi.value = x;
                Xi.lastModificationIteration = currentIteration - 1;
            }
            Xi.backup();
            Xi.value = Mutate(uniformFloat(), Xi.value, s1, s2);
#else
            Xi.backup();
            // nSmall mutations is equivalent to normal distribution of n sigma^2
            const Float sigma = 0.02f;
            const Float sqrt2 = std::sqrt(2.0f);
            // importance sample N(Xi, n sigma^2)
            Float normalSample = sqrt2 * ErfInv(2 * uniformFloat() - 1);
            Float effSigma = sigma * std::sqrt((Float) nSmall);
            Xi.value += normalSample * effSigma;
            Xi.value -= std::floor(Xi.value);
#endif
        }
        Xi.lastModificationIteration = currentIteration;
    }
}
#endif //MIYUKI_MLTSAMPLER_H
