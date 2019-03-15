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