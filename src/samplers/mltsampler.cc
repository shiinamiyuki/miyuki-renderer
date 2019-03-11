//
// Created by Shiina Miyuki on 2019/3/9.
//

#include "mltsampler.h"

namespace Miyuki {
    inline Float mutate(Float random, Float value) {
        const float S1 = 1.0f / 1024.0f;
        const float S2 = 1.0f / 64.0f;
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

    void MLTSampler::accept() {
        if (largeStep) {
            lastLargeStepIteration = currentIteration;
        }
    }

    void MLTSampler::reject() {
        for (PrimarySample &Xi :X) {
            if (Xi.lastModificationIteration == currentIteration) {
                Xi.restore();
            }
        }
        --currentIteration;
    }

    int MLTSampler::getNextIndex() {
        return streamIndex + nStream * sampleIndex++;
    }

    void MLTSampler::ensureReady(int index) {
        if (index >= X.size())
            X.resize(index + 1u);
        auto &Xi = X[index];
        if (Xi.lastModificationIteration < lastLargeStepIteration) {
            Xi.value = uniformInt32();
            Xi.lastModificationIteration = lastLargeStepIteration;
        }

        if (largeStep) {
            Xi.backup();
            Xi.value = uniformFloat();
        } else {
            int64_t nSmall = currentIteration - Xi.lastModificationIteration;
            auto nSmallMinus = nSmall - 1;
            if (nSmallMinus > 0) {
                auto x = Xi.value;
                while (nSmallMinus > 0) {
                    nSmallMinus--;
                    x = mutate(uniformFloat(), x);
                }
                Xi.value = x;
                Xi.lastModificationIteration = currentIteration - 1;
            }
            Xi.backup();
            Xi.value = mutate(uniformFloat(), Xi.value);
        }
        Xi.lastModificationIteration = currentIteration;
    }

    void MLTSampler::start() {

    }

    void MLTSampler::startIteration() {
        currentIteration++;
        largeStep = uniformFloat() < largeStepProbability;
    }
}