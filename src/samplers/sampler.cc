//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "sampler.h"

namespace Miyuki {

    int32_t RNG::uniformInt32() {
        return LCRandI(seed);
    }

    Float RNG::uniformFloat() {
        return LCRand(seed);
    }

    int32_t RNG::uniformInt32(Seed *seed) {
        return LCRandI(seed);
    }

    Float RNG::uniformFloat(Seed *seed) {
        return LCRand(seed);
    }

    void RandomSampler::start() {

    }

    Float RandomSampler::get1D() {
        return rng.uniformFloat();
    }

    Point2f RandomSampler::get2D() {
        return {get1D(), get1D()};
    }
}