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

#ifndef MIYUKIRENDERER_SAMPLER_H
#define MIYUKIRENDERER_SAMPLER_H

#include <api/object.hpp>
#include <api/math.hpp>

namespace miyuki::core {
    class Sampler : public Object {
    public:
        virtual void startPixel(const Point2i &, const Point2i &filmDimension) = 0;

        virtual Float next1D() = 0;

        virtual Point2f next2D() {
            return Point2f(next1D(), next1D());
        }

        virtual void startNextSample() = 0;

        [[nodiscard]] virtual std::shared_ptr<Sampler> clone() const = 0;
    };
}

#endif //MIYUKIRENDERER_SAMPLER_H
