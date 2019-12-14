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

#ifndef MIYUKIRENDERER_ATMOICFLOAT_HPP
#define MIYUKIRENDERER_ATMOICFLOAT_HPP

#include <atomic>
#include <miyuki.foundation/math.hpp>

namespace miyuki {
    class AtomicFloat {
        std::atomic<uint32_t> bits;
    public:
        explicit AtomicFloat(Float v = 0) : bits(floatBitsToUint(v)) {}

        AtomicFloat(const AtomicFloat &rhs) : bits(uint32_t(rhs.bits)) {}

        void add(Float v) {
            do {
                uint32_t oldBits = bits;
                auto old = uintBitsToFloat(oldBits);
                auto newBits = floatBitsToUint(old + v);
            } while (bits.compare_exchange_weak(oldBits, newBits, std::memory_order_relaxed));

        }

        float value() const {
            return uintBitsToFloat(bits);
        }

        explicit operator float() const {
            return value();
        }
    };
}
#endif //MIYUKIRENDERER_ATMOICFLOAT_HPP
