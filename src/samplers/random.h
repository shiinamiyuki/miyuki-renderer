//
// Created by Shiina Miyuki on 2019/1/17.
//

#ifndef MIYUKI_RANDOM_H
#define MIYUKI_RANDOM_H

#include "sampler.h"
#include "../math/geometry.h"

namespace Miyuki {
    class RandomSampler : public Sampler {
    public:
        RandomSampler(Seed *s) : Sampler(s) {}

        Float nextFloat() override;

        int32_t nextInt() override;

        Float nextFloat(Seed *seed) override;

        int32_t nextInt(Seed *seed) override;

        Point2f nextFloat2D() override;
    };
}
#endif //MIYUKI_RANDOM_H
