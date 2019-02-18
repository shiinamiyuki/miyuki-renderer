//
// Created by Shiina Miyuki on 2019/2/18.
//

#include "mmlt.h"
#include "../../math/func.h"

using namespace Miyuki;

Miyuki::Float Miyuki::MLTSampler::nextFloat() {
    int index = getNextIndex();
    ensureReady(index);
    return X[index].value;
}

Miyuki::Point2f Miyuki::MLTSampler::nextFloat2D() {
    return {nextFloat(), nextFloat()};
}

void MLTSampler::ensureReady(int index) {
    if (index >= X.size())
        X.resize(index + 1u);
    auto &Xi = X[index];
    if (Xi.lastModificationIteration < lastLargeStepIteration) {
        Xi.value = randFloat();
        Xi.lastModificationIteration = lastLargeStepIteration;
    }
    Xi.backup();
    if (largeStep) {
        Xi.value = randFloat();
    } else {
        // Applying n small mutations of N(u1, sigma) is equivalent to sampling N(X, n*sigma^2)
        // TODO: why? Justify the math
        int64_t nSmall = currentIteration - Xi.lastModificationIteration;
        const Float Sqrt2 = std::sqrt(2);
        Float normalSample = Sqrt2 * erfInv(2 * randFloat() - 1);
        Float effSigma = sigma * std::sqrt((Float) nSmall);
        Xi.value += normalSample * effSigma;
        Xi.value -= std::floor(Xi.value);
    }
    Xi.lastModificationIteration = currentIteration;
}

void MLTSampler::start() {

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
    return streamIndex + streamCount * sampleIndex++;
}

void MLTSampler::startIteration() {
    currentIteration++;
    largeStep = randFloat() < largeStepProbability;
}

void MLTSampler::startStream(int index) {
    streamIndex = index;
}

void MultiplexedMLT::iteration(Scene &scene) {
    BDPT::iteration(scene);
}

void MultiplexedMLT::render(Scene &scene) {
    BDPT::render(scene);
}
