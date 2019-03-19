//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "sampler.h"

namespace Miyuki {

    int32_t RNG::uniformInt32() {
        return xorshift64star(seed) & UINT32_MAX;
    }

    Float RNG::uniformFloat() {
        return xorshift64star(seed) / (float) UINT64_MAX;
    }

    int32_t RNG::uniformInt32(Seed *seed) {
        return xorshift64star(seed) & UINT32_MAX;
    }

    Float RNG::uniformFloat(Seed *seed) {
        return xorshift64star(seed) / (float) UINT64_MAX;
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