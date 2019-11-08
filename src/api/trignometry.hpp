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
#pragma once

#include <api/math.hpp>

namespace miyuki {

    inline Float CosTheta(const Vec3f &w) { return w.y; }
    inline Float AbsCosTheta(const Vec3f &w) { return std::abs(CosTheta(w)); }
    inline Float Cos2Theta(const Vec3f &w) { return w.y * w.y; }
    inline Float Sin2Theta(const Vec3f &w) { return 1 - Cos2Theta(w); }
    inline Float SinTheta(const Vec3f &w) { return std::sqrt(std::fmax(0.0f, Sin2Theta(w))); }
    inline Float Tan2Theta(const Vec3f &w) { return Sin2Theta(w) / Cos2Theta(w); }
    inline Float TanTheta(const Vec3f &w) { return std::sqrt(std::fmax(0.0f, Tan2Theta(w))); }
    inline bool SameHemisphere(const Vec3f &wo, const Vec3f &wi) { return wo.y * wi.y >= 0; }

    inline Vec3f Reflect(const Vec3f &w, const Normal3f &n) { return -w + 2.0f * w.dot(n) * n; }

} // namespace miyuki