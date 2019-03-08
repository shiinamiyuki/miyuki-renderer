//
// Created by Shiina Miyuki on 2019/3/5.
//

#include "sobol.h"
#include "thirdparty/sobol/sobol.hpp"

namespace Miyuki {
    static Float *sobolValues = nullptr;
    const static int maxN = 1024 * 1024;
    static int N = 0;
    static int D = 0;

    static void init() {
        N = maxN / D;
        delete[] sobolValues;
        sobolValues = i4_sobol_generate(D, N, rand() % N);
    }

    void SobolSampler::start() {
        sobolIndex++;
        if (sobolIndex >= N)
            sobolIndex = 0;
        sampleIndex = 0;
    }

    Float SobolSampler::get1D() {
        return sobolValues[sobolIndex * D + sampleIndex++];
    }

    Point2f SobolSampler::get2D() {
        return {get1D(), get1D()};
    }

    SobolSampler::SobolSampler(Seed *s, int dimension) : Sampler(s), dimension(dimension) {
        if (!sobolValues || D != dimension) {
            D = dimension;
            init();
        }
        sobolIndex = uniformInt32() % N;
    }
}
