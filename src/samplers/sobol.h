//
// Created by Shiina Miyuki on 2019/3/5.
//

#ifndef MIYUKI_SOBOL_H
#define MIYUKI_SOBOL_H

#include "sampler.h"

namespace Miyuki {
    class SobolSampler : public Sampler {
        int dimension;
        uint32_t rng;
        uint32_t numPass;
    public:
        SobolSampler(Seed *s);

        void start() override;

        Float get1D() override;

        Point2f get2D() override;
    };
}
#endif //MIYUKI_SOBOL_H
