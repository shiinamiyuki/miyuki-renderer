//
// Created by Shiina Miyuki on 2019/2/14.
//

#include "sobol.h"
#include "../../thirdparty/sobol/sobol.hpp"

using namespace Miyuki;
static Float *sobolMatrix = nullptr;
static int sobolDimension = -1;
static int N = 1024 * 1024;

void init(int dim) {
    if (dim != sobolDimension) {
        fmt::print("Generating sobol matrix\n");
        delete[] sobolMatrix;
        sobolDimension = dim;
        sobolMatrix = i4_sobol_generate(sobolDimension, N, 0);
    }
}

Float SobolSampler::nextFloat() {
    if (sobolIndex >= N)
        sobolIndex = 0;
    int idx = sobolIndex * sobolDimension + dimIndex++;
    return sobolMatrix[idx];
}

int32_t SobolSampler::nextInt() {
    return randInt();
}

Float SobolSampler::nextFloat(Seed *seed) {
    return nextFloat();
}

int32_t SobolSampler::nextInt(Seed *seed) {
    return nrand48(seed->getPtr());
}

Point2f SobolSampler::nextFloat2D() {
    return {nextFloat(), nextFloat()};
}

SobolSampler::SobolSampler(Seed *s, int dim) : Sampler(s), dim(dim) {
    sobolIndex = randInt() % N;
    init(dim);
}

static std::mutex mutex;

void SobolSampler::start() {
    sobolIndex++;
    dimIndex = 0;

}
