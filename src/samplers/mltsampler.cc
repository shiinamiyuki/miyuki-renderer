//
// Created by Shiina Miyuki on 2019/3/9.
//

#include "mltsampler.h"


namespace Miyuki {
    void MLTSampler::accept() {
        if (large()) {
            largeAcceptCount++;
        } else {
            smallAcceptCount++;
        }
        if (largeStep) {
            lastLargeStepIteration = currentIteration;
        }
        rejectCount = 0;
    }

    void MLTSampler::reject() {
        if (u1.lastModificationIteration == currentIteration)
            u1.restore();
        if (u2.lastModificationIteration == currentIteration)
            u2.restore();
        rejectCount++;
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
        if(rejectCount >= maxConsecutiveRejects)
            largeStep = true;
        if (largeStep) {
            largeCount++;
        } else {
            smallCount++;
        }
    }
}