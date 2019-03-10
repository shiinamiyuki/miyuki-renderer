//
// Created by Shiina Miyuki on 2019/3/5.
//

#include "sobol.h"
#include "sobolmat.hpp"

static inline uint32_t cmj_hash_simple(uint32_t i, uint32_t p) {
    i = (i ^ 61) ^ p;
    i += i << 3;
    i ^= i >> 4;
    i *= 0x27d4eb2d;
    return i;
}

#define SOBOL_SKIP 64

static inline float sobol(const unsigned int vectors[][32], unsigned int dimension, unsigned int i,unsigned int rng) {
    unsigned int result = 0;
    i += SOBOL_SKIP;
    for (unsigned int j = 0; i; i >>= 1, j++)
        if (i & 1)
            result ^= vectors[dimension][j];

    float r =  result * (1.0f / (float) 0xFFFFFFFF);
    uint32_t tmp_rng = cmj_hash_simple(dimension, rng);
    float shift = tmp_rng * (1.0f / (float) 0xFFFFFFFF);
    return r + shift - floorf(r + shift);
}

namespace Miyuki {
    void SobolSampler::start() {
        dimension = 0;
        numPass++;
    }

    Float SobolSampler::get1D() {
        return sobol(SobolMatrix, dimension++, numPass, rng);
    }

    Point2f SobolSampler::get2D() {
        return {get1D(), get1D()};
    }

    SobolSampler::SobolSampler(Seed *s) : Sampler(s) {
        numPass = 0;
        rng = static_cast<uint32_t>(uniformInt32());
    }

}
