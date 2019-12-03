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

#ifndef MIYUKIRENDERER_SAMPLING_H
#define MIYUKIRENDERER_SAMPLING_H

#include <miyuki.foundation/math.hpp>

namespace miyuki::core {
    inline Point2f ConcentricSampleDisk(const Point2f &u) {
        Point2f uOffset = 2.f * u - Point2f(1, 1);
        if (uOffset.x == 0 && uOffset.y == 0)
            return Point2f(0, 0);

        Float theta, r;
        if (std::abs(uOffset.x) > std::abs(uOffset.y)) {
            r = uOffset.x;
            theta = Pi4 * (uOffset.y / uOffset.x);
        } else {
            r = uOffset.y;
            theta = Pi2 - Pi4 * (uOffset.x / uOffset.y);
        }
        return r * Point2f(std::cos(theta), std::sin(theta));
    }

    inline Vec3f CosineHemisphereSampling(const Point2f &u) {
        auto uv = ConcentricSampleDisk(u);
        auto r = dot(uv, uv);
        auto h = std::sqrt(1 - r);
        return Vec3f(uv.x, h, uv.y);
    }
}
#endif //MIYUKIRENDERER_SAMPLING_H
