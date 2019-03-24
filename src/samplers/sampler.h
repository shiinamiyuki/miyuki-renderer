//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_SAMPLER_H
#define MIYUKI_SAMPLER_H

#include <cameras/camera.h>

#include "miyuki.h"
#include "core/geometry.h"

namespace Miyuki {
    typedef uint64_t Seed;

    inline uint64_t xorshift64star(uint64_t *state) {
        uint64_t x = state[0];    /* The state must be seeded with a nonzero value. */
        x ^= x >> 12; // a
        x ^= x << 25; // b
        x ^= x >> 27; // c
        state[0] = x;
        return x * 0x2545F4914F6CDD1D;
    }

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

        Seed *getSeed() { return seed; }

        Sampler(Seed *seed) : seed(seed), rng(seed) {}

        void reseed(Seed *seed) {
            this->seed = seed;
            rng = RNG(seed);
        }

        virtual void start() = 0;

        virtual Float get1D() = 0;

        virtual Point2f get2D() = 0;

        uint32_t uniformInt32() {
            return rng.uniformInt32();
        }

        Float uniformFloat() {
            return rng.uniformFloat();
        }

        virtual void startPass(uint32_t N) {}

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
