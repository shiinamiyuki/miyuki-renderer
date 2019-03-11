//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_SAMPLER_H
#define MIYUKI_SAMPLER_H

#include <cameras/camera.h>

#include "miyuki.h"
#include "core/geometry.h"

namespace Miyuki {
    struct Seed {
        uint16_t seeds[3];

        Seed();

        uint16_t &operator[](uint32_t i) {
            Assert(i < 3);
            return seeds[i];
        }

        uint16_t *get() {
            return seeds;
        }
    };

    class RNG {
        Seed *seed;
    public:
        RNG(Seed *seed = nullptr) : seed(seed) {}

        void reseed(Seed *seed) { this->seed = seed; }

        int32_t uniformInt32();

        Float uniformFloat();

        int32_t uniformInt32(Seed *seed);

        Float uniformFloat(Seed *seed);
    };

    class Sampler {
    protected:
        Seed *seed;
        RNG rng;
    public:
        Sampler(Seed *seed) : seed(seed), rng(seed) {}

        virtual void start() = 0;

        virtual Float get1D() = 0;

        virtual Point2f get2D() = 0;

        int32_t uniformInt32() {
            return rng.uniformInt32();
        }

        Float uniformFloat() {
            return rng.uniformFloat();
        }

        virtual void startDimension(int dimension) {}
    };

    class RandomSampler : public Sampler {
    public:
        RandomSampler(Seed *seed) : Sampler(seed) {}

        void start() override;

        Float get1D() override;

        Point2f get2D() override;
    };
}

#endif //MIYUKI_SAMPLER_H
