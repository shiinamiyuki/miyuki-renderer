//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_THREAD_H
#define MIYUKI_THREAD_H

#include "miyuki.h"
#include "core/geometry.h"

namespace Miyuki {
    namespace Thread {
        using TaskFunc = std::function<void(uint32_t idx, uint32_t threadId)>;
        using TaskFunc2D = std::function<void(Point2i idx, uint32_t threadId)>;

        void ParallelFor(uint32_t begin, uint32_t end, TaskFunc, size_t workSize=1);

        void ParallelFor2D(Point2i N, TaskFunc2D);

        class WorkGroup {
            uint32_t threadId;
            uint32_t begin, end;
            TaskFunc task;
        public:
            WorkGroup(uint32_t begin, uint32_t end, TaskFunc func)
                    : threadId(-1), begin(begin), end(end), task(func) {}

            void invoke(uint32_t id);
        };

        class ThreadPool {
        private:
            std::mutex taskMutex, mainMutex;
            std::condition_variable taskCondition, mainCondition;
            std::vector<std::unique_ptr<std::thread>> workers;
            std::deque<WorkGroup> workList;
            std::atomic<bool> terminated;
            std::atomic<int> activeWorkers;
            std::atomic<bool> mainWaiting;

            void workThreadFunc(uint32_t id);

            friend void parallelFor(uint32_t begin, uint32_t end, TaskFunc);

            friend void parallelFor2D(Point2i N, TaskFunc2D);

        public:
            ThreadPool();

            uint32_t numThreads() const;

            void enqueue(TaskFunc, uint32_t begin = 0, uint32_t end = 1u);


            void reset();

            void stop();

            void waitForAll();
        };

        extern ThreadPool *pool;

    }
}
#endif //MIYUKI_THREAD_H
