//
// Created by Shiina Miyuki on 2019/3/7.
//

#ifndef MIYUKI_PROGRESS_H
#define MIYUKI_PROGRESS_H

#include "miyuki.h"
#include "profile.h"

namespace Miyuki {
    class ProgressReporter {
        std::function<void(int, int)> callback;
        int total;
        std::atomic<int> counter;
        Profiler profiler;
    public:
        ProgressReporter(int total, std::function<void(int, int)> callback)
                : callback(std::move(callback)), counter(0), total(total) {}

        void update();

        Float percentage() const {
            return (Float) counter / total;
        }

        Float elapsedSeconds() const {
            return profiler.elapsedSeconds();
        }

        Float estimatedTimeToFinish() const;

        int count() const { return counter; }
    };
}
#endif //MIYUKI_PROGRESS_H
