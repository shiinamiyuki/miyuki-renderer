//
// Created by Shiina Miyuki on 2019/2/28.
//

#include <utils/thread.h>

namespace Miyuki {

    namespace Thread {
        ThreadPool *pool = new ThreadPool();

        ThreadPool::ThreadPool() : terminated(false), activeWorkers(0), mainWaiting(false) {
			hardwareConcurrency = std::thread::hardware_concurrency();
            for (int i = 0; i < numThreads(); i++) {
                workers.emplace_back(std::make_unique<std::thread>([=]() {
                    workThreadFunc(i);
                }));
            }
        }

        void ThreadPool::enqueue(TaskFunc func, uint32_t start, uint32_t end) {
            std::lock_guard<std::mutex> lock(taskMutex);
            workList.emplace_back(start, end, func);
            taskCondition.notify_all();
        }

        void ThreadPool::reset() {
            throw NotImplemented();
        }

        void ThreadPool::stop() {
            terminated = true;
            {
                std::unique_lock<std::mutex> lock(taskMutex);
                taskCondition.notify_all();
            }
            while (!workers.empty()) {
                workers.back()->detach();
                workers.pop_back();
            }
        }

        void ParallelFor(uint32_t begin, uint32_t end, TaskFunc task, size_t workSize) {
            while (begin + workSize < end) {
                pool->enqueue(task, begin, begin + workSize);
                begin += workSize;
            }
            if (begin < end) {
                pool->enqueue(task, begin, end);
            }
            pool->waitForAll();
        }

        void ParallelFor2D(Point2i N, TaskFunc2D taskFunc2D, size_t workSize) {
            ParallelFor(0u, N.x() * N.y(), [=](uint32_t id, uint32_t threadId) {
                // id = x + N.x() * y
                int x = id % N.x();
                int y = id / N.x();
                taskFunc2D(Point2i{x, y}, threadId);
            }, workSize);
        }
		
        uint32_t ThreadPool::numThreads() const {
            return hardwareConcurrency;
        }

        void ThreadPool::workThreadFunc(uint32_t id) {
            std::unique_lock<std::mutex> lock(taskMutex);
            while (!terminated) {
                if (workList.empty()) {
                    taskCondition.wait(lock);
                } else {
                    activeWorkers++;
                    auto work = workList.front();
                    workList.pop_front();
                    lock.unlock();
                    work.invoke(id);
                    activeWorkers--;
                    lock.lock();
                    if (activeWorkers == 0) {
                        std::unique_lock<std::mutex> _lock(mainMutex);
                        if (mainWaiting)
                            mainCondition.notify_one();
                    }
                }
            }
        }

        void ThreadPool::waitForAll() {
            if (mainWaiting)
                return;
            std::unique_lock lock(mainMutex);
            mainWaiting = true;
            mainCondition.wait(lock);
            mainWaiting = false;
        }


        void WorkGroup::invoke(uint32_t id) {
            threadId = id;
            for (auto i = begin; i < end; i++) {
                task(i, threadId);
            }
        }
    }
}