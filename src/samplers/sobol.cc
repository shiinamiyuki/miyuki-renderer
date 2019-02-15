//
// Created by Shiina Miyuki on 2019/2/14.
//

#include "sobol.h"
#include "../../thirdparty/sobol/sobol.hpp"

using namespace Miyuki;

Float SobolSampler::nextFloat() {
    return randFloat();
}

int32_t SobolSampler::nextInt() {
    return randInt();
}

Float SobolSampler::nextFloat(Seed *seed) {
    return erand48(seed->getPtr());
}

int32_t SobolSampler::nextInt(Seed *seed) {
    return nrand48(seed->getPtr());
}

Point2f SobolSampler::nextFloat2D() {
    Float v[2];
    i4_sobol(2, &sobolIndex, v);
    return {v[0], v[1]};
}

SobolSampler::SobolSampler(Seed *s) : Sampler(s) {
    sobolIndex = randInt() % (1024);
}
