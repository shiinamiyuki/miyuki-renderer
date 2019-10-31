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

#ifndef MIYUKIRENDERER_SOBOL_SAMPLER_H
#define MIYUKIRENDERER_SOBOL_SAMPLER_H

#include <api/sampler.h>
#include <api/serialize.hpp>

namespace miyuki::core {
    class SobolSampler final: public Sampler {
        int dimension = 0;
        int sample = 0;
        int rotation;
    public:
        MYK_DECL_CLASS(SobolSampler, "SobolSampler", interface = "Sampler")

        void startPixel(const Point2i &i, const Point2i &filmDimension) override;

        Float next1D() override;

        void startNextSample() override;

        [[nodiscard]] std::shared_ptr<Sampler> clone() const override;
    };
}

#endif //MIYUKIRENDERER_SOBOL_SAMPLER_H
