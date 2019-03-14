//
// Created by Shiina Miyuki on 2019/3/9.
//

#include "mltsampler.h"

namespace Miyuki {
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

    void MLTSampler::mutate(PrimarySample &Xi, Float s1, Float s2) {
        if (Xi.lastModificationIteration < lastLargeStepIteration) {
            Xi.value = uniformFloat();
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
                    x = Mutate(uniformFloat(), x, s1, s2);
                }
                Xi.value = x;
                Xi.lastModificationIteration = currentIteration - 1;
            }
            Xi.backup();
            Xi.value = Mutate(uniformFloat(), Xi.value, s1, s2);
        }
        Xi.lastModificationIteration = currentIteration;
    }

    void MLTSampler::ensureReady(int index) {
        if (index >= X.size())
            X.resize(index + 1u);
        auto &Xi = X[index];
        mutate(Xi, 1.0f / 1024.0f, 1.0f / 64.0f);
    }

    void MLTSampler::start() {

    }

    void MLTSampler::startIteration() {
        currentIteration++;
        largeStep = uniformFloat() < largeStepProbability;
    }


}