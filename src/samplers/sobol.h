//
// Created by Shiina Miyuki on 2019/3/5.
//

#ifndef MIYUKI_SOBOL_H
#define MIYUKI_SOBOL_H

#include "sampler.h"
namespace Miyuki{
    class SobolSampler : public Sampler{
        int dimension;
        int sobolIndex;
        int sampleIndex;
    public:
        SobolSampler(Seed *s,int dimension);
        void start() override;

        Float get1D() override;

        Point2f get2D() override;
    };
    void InitSobolSamples(int M);
}
#endif //MIYUKI_SOBOL_H
