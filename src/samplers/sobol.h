//
// Created by Shiina Miyuki on 2019/2/14.
//

#ifndef MIYUKI_SOBOL_H
#define MIYUKI_SOBOL_H

#include "sampler.h"
namespace Miyuki{
    class SobolSampler : public Sampler{
        int sobolIndex;
    public:
        SobolSampler(Seed * s);
        Float nextFloat() override;

        int32_t nextInt() override;

        Float nextFloat(Seed *seed) override;

        int32_t nextInt(Seed *seed) override;

        Point2f nextFloat2D() override;
    };
}
#endif //MIYUKI_SOBOL_H
