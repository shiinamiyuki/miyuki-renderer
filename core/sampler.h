//
// Created by Shiina Miyuki on 2019/1/17.
//

#ifndef MIYUKI_SAMPLER_H
#define MIYUKI_SAMPLER_H

#include "util.h"
#include "geometry.h"
namespace Miyuki {
    class Seed {
        unsigned short Xi[3];
    public:
        using Type = unsigned short;

        Seed() { Xi[2] = rand(); }

        unsigned short &operator[](unsigned int i) {
            assert(i < 3);
            return Xi[i];
        }

        Type *getPtr() { return Xi; }
    };

    class Sampler {
    protected:
        Seed *seed;
    public:
        explicit Sampler(Seed *s) : seed(s) {}

        Seed *getSeed() const { return seed; }

        void setSeed(Seed *s) { seed = s; }

        virtual Float nextFloat() = 0;

        virtual int nextInt() = 0;

        virtual Float nextFloat(Seed *) = 0;

        virtual int nextInt(Seed *) = 0;

        virtual Point2f nextFloat2D() = 0;

        virtual void start() {}

        int randInt() {
            return nrand48(seed->getPtr());
        }

        Float randFloat() {
            return erand48(seed->getPtr());
        }
    };
}
#endif //MIYUKI_SAMPLER_H
