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

#include <api/parallel.h>
#include <cstdint>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <deque>
#include <algorithm>

namespace miyuki {
    class ParallelForContext {
        std::vector<std::thread> workers;
        std::mutex taskMutex, mainMutex;
        std::condition_variable mainWaiting, taskWaiting;
        std::atomic<bool> shutdown;
        std::deque<std::pair<int64_t, int64_t >> queue;
        std::atomic<uint32_t> activeWorkers;
        WorkFunc workFunc;
    public:
        ParallelForContext() noexcept : workers(GetCoreNumber()), shutdown(false), activeWorkers(0) {
            for (int i = 0; i < workers.size(); i++) {
                workers[i] = std::move(std::thread([=]() {
                    uint32_t threadId = i;
                    std::unique_lock<std::mutex> lock(taskMutex);
                    while (!shutdown) {
                        if (queue.empty()) {
                            {
                                std::unique_lock<std::mutex> lock1(mainMutex);
                                mainWaiting.notify_one();
                            }
                            taskWaiting.wait(lock);
                        } else {
                            activeWorkers++;

                            auto work = queue.front();
                            queue.pop_front();

                            lock.unlock();
                            for (auto index = work.first; index < work.second; index++) {
                                workFunc(index, threadId);
                            }
                            lock.lock();
                            activeWorkers--;
                        }
                    }
                }));
            }
        }

        ~ParallelForContext() {
            shutdown = true;
            taskWaiting.notify_all();

            for (auto &i:workers) {
                if (i.joinable()) {

                    i.join();
                }
            }
        }


        void parallelFor(int64_t begin, int64_t end, WorkFunc func, size_t workSize) {
            {
                std::unique_lock<std::mutex> lock(taskMutex);
                workFunc = std::move(func);
                while (begin < end) {
                    auto lo = begin;
                    auto hi = std::min<size_t>(end, begin + workSize);
                    queue.emplace_back(lo, hi);
                    begin += workSize;
                }

                taskWaiting.notify_all();
            }
            {
                std::unique_lock<std::mutex> lock(mainMutex);
                mainWaiting.wait(lock);
            }
        }

    };

    static size_t CoreNumber = std::thread::hardware_concurrency();
    static ParallelForContext parallelForContext;

    void ParallelFor(int64_t begin, int64_t end, WorkFunc func, size_t workSize) {
        parallelForContext.parallelFor(begin, end, std::move(func), workSize);
    }


    size_t GetCoreNumber() {
        return CoreNumber;
    }

    void SetCoreNumber(size_t N) {
        CoreNumber = N;
    }

}