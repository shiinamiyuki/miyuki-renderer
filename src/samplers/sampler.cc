//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "sampler.h"

namespace Miyuki {

    int32_t RNG::uniformInt32() {
        return nrand48(seed->get());
    }

    Float RNG::uniformFloat() {
        return static_cast<Float>(erand48(seed->get()));
    }

    int32_t RNG::uniformInt32(Seed *seed) {
        return nrand48(seed->get());
    }

    Float RNG::uniformFloat(Seed *seed) {
        return static_cast<Float>(erand48(seed->get()));
    }

    void RandomSampler::start() {

    }

    Float RandomSampler::get1D() {
        return rng.uniformFloat();
    }

    Point2f RandomSampler::get2D() {
        return {get1D(), get1D()};
    }

    Seed::Seed() {
        static std::random_device rd;
        static std::uniform_int_distribution<int16_t> dist;
        for (int i = 0; i < 3; i++)
            seeds[i] = dist(rd);
    }
}