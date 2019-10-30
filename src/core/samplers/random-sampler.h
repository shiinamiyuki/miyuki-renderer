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

#ifndef MIYUKIRENDERER_RANDOM_SAMPLER_H
#define MIYUKIRENDERER_RANDOM_SAMPLER_H

#include <api/rng.h>
#include <api/sampler.h>
#include <api/serialize.hpp>

namespace miyuki::core {
    class RandomSampler final : public Sampler {
        Rng rng;
    public:
        MYK_DECL_CLASS(RandomSampler, "RandomSampler", interface = "Sampler")

        RandomSampler(uint32_t seed = 0) : rng(seed) {}

        void startPixel(const Point2i &i, const Point2i &filmDimension) override {
            rng = Rng(i.x + i.y * filmDimension.x);
        }

        Float next1D() override {
            return rng.uniformFloat();
        }

        [[nodiscard]] std::shared_ptr<Sampler> clone() const override {
            return std::make_shared<RandomSampler>();
        }
    };

}

#endif //MIYUKIRENDERER_RANDOM_SAMPLER_H
