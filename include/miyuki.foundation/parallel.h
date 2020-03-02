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

#ifndef MIYUKIRENDERER_PARALLEL_H
#define MIYUKIRENDERER_PARALLEL_H

#include <functional>
#include <thread>

namespace miyuki {
    using WorkFunc = std::function<void(int64_t index, size_t threadIdx)>;

    size_t GetCoreNumber();

    void SetCoreNumber(size_t N);

    void ParallelFor(int64_t begin, int64_t end, WorkFunc, size_t workSize = 1);

    template<class F1, class F2>
    void ParallelDo(F1 &&f1, F2 &&f2) {
        std::thread thread(f2);
        f1();
        f2.join();
    }
}
#endif //MIYUKIRENDERER_PARALLEL_H
