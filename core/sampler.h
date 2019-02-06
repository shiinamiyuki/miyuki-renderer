//
// Created by Shiina Miyuki on 2019/1/17.
//

#ifndef MIYUKI_SAMPLER_H
#define MIYUKI_SAMPLER_H

#include "util.h"
#include "geometry.h"

namespace Miyuki {
    template<uint32_t Dividend>
    uint32_t divide(uint32_t x) {
        constexpr uint64_t k = ((0x100000000LL) / Dividend);
        uint64_t t = k * x;
        return t >> 33;
    }

    class Seed {
        unsigned short Xi[3];
    public:
        using Type = unsigned short;

        Seed() { Xi[2] = rand(); }

        unsigned short &operator[](uint32_t i) {
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

        virtual int32_t nextInt() = 0;

        virtual Float nextFloat(Seed *) = 0;

        virtual int32_t nextInt(Seed *) = 0;

        virtual Point2f nextFloat2D() = 0;

        virtual void start() {}

        int32_t randInt() {
            return nrand48(seed->getPtr());
        }

        Float randFloat() {
            return erand48(seed->getPtr());
        }
    };

    inline Float balanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
        return (nf * fPdf) / (nf * fPdf + ng * gPdf);
    }

    inline Float powerHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
        Float f = nf * fPdf, g = ng * gPdf;
        return (f * f) / (f * f + g * g);
    }
}
#endif //MIYUKI_SAMPLER_H
