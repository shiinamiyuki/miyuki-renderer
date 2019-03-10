//
// Created by Shiina Miyuki on 2019/3/5.
//

#include "sobol.h"
#include "sobolmat.hpp"
#define SOBOL_SKIP 64
static float sobol(const unsigned int vectors[][32], unsigned int dimension, unsigned int i)
{
    unsigned int result = 0;
    for(unsigned int j = 0; i; i >>= 1, j++)
        if(i & 1)
            result ^= vectors[dimension][j];

    return result * (1.0f/(float)0xFFFFFFFF);
}

namespace Miyuki {
    void SobolSampler::start() {
        dimension = 0;
        numPass++;
    }

    Float SobolSampler::get1D() {
        return sobol(SobolMatrix, dimension++, numPass);
    }

    Point2f SobolSampler::get2D() {
        return {get1D(), get1D()};
    }

    SobolSampler::SobolSampler(Seed *s) : Sampler(s) {
        numPass = uniformInt32();
    }

}
