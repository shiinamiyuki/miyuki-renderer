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

#ifndef MIYUKIRENDERER_FUNCS_H
#define MIYUKIRENDERER_FUNCS_H
namespace miyuki {

    template<int N, typename T>
    T powN(const T &v) {
        if constexpr (N == 0) {
            return T(1.0);
        } else if constexpr(N == 1) {
            return v;
        } else if constexpr (N % 2 == 0) {
            auto t = powN<N / 2, T>(v);
            return t * t;
        } else {
            auto t = powN<N / 2, T>(v);
            return t * t * v;
        }
    }

    float uintBitsToFloat(const uint32_t &i) {
        float v;
        std::memcpy(&v, &i, sizeof(float));
        return v;
    }

    uint32_t floatBitsToUint(const float &v) {
        uint32_t i;
        std::memcpy(&i, &v, sizeof(uint32_t));
        return i;
    }
}
#endif //MIYUKIRENDERER_FUNCS_H
