// MIT License
//
// Copyright (c) 2019 椎名深雪
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <api/rng.h>
#include "sobol-sampler.h"
#include "sobolmat.hpp"


namespace miyuki::core {
#define SOBOL_SKIP 64

    static inline uint32_t cmj_hash_simple(uint32_t i, uint32_t p) {
        i = (i ^ 61) ^ p;
        i += i << 3;
        i ^= i >> 4;
        i *= 0x27d4eb2d;
        return i;
    }

    // https://wiki.blender.org/wiki/Source/Render/Cycles/Sobol
    static inline float
    sobol(const unsigned int vectors[][32], unsigned int dimension, unsigned int i, unsigned int rng) {
        unsigned int result = 0;
        i += SOBOL_SKIP;
        for (unsigned int j = 0; i; i >>= 1, j++)
            if (i & 1)
                result ^= vectors[dimension][j];

        float r = result * (1.0f / (float) 0xFFFFFFFF);
        uint32_t tmp_rng = cmj_hash_simple(dimension, rng);
        float shift = tmp_rng * (1.0f / (float) 0xFFFFFFFF);
        return r + shift - floorf(r + shift);
    }

    void SobolSampler::startPixel(const Point2i &i, const Point2i &filmDimension) {
        Rng rng(i.x + i.y * filmDimension.x);
        rotation = rng.uniformUint32();
        sample = -1;
    }

    Float SobolSampler::next1D() {
        return sobol(SobolMatrix, dimension++, sample, rotation);
    }

    std::shared_ptr<Sampler> SobolSampler::clone() const {
        return std::make_shared<SobolSampler>();
    }

    void SobolSampler::startNextSample() {
        dimension = 0;
        sample++;
    }
}

