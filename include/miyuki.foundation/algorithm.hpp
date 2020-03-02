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
#include <miyuki.foundation/defs.h>
#include <cstddef>

namespace miyuki {
    // find i such that arr[i] <= val < arr[i+1]
    template <class T> T *FindInterval(T arr, size_t begin, size_t end, const T &val) {
        auto low = 0;
        auto hi = end - 1;
        size_t mid = low;
        while (low <= hi) {
            mid = (low + hi) / 2;
            if (mid + 1 >= end || arr[mid] <= val && val < arr[mid + 1]) {
                return mid;
            }
            if (arr[mid] > val) {
                hi = mid - 1;
            } else {
                low = mid + 1;
            }
        }
        return -1;
    }
} // namespace miyuki