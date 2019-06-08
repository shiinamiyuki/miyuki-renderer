//
// Created by Shiina Miyuki on 2019/3/7.
//

#ifndef MIYUKI_PROGRESS_H
#define MIYUKI_PROGRESS_H

#include "miyuki.h"
#include "profile.h"

namespace Miyuki {
    template<typename T>
    class ProgressReporter {
        std::function<void(T, T)> callback;
        T total;
        std::atomic<T> counter;
        Profiler profiler;
    public:
        ProgressReporter(T total, std::function<void(T, T)> callback)
                : callback(std::move(callback)), counter(0), total(total) {}

        double percentage() const {
            return (double) counter / total;
        }

        double elapsedSeconds() const {
            return profiler.elapsedSeconds();
        }

        T count() const { return counter; }

        void update() {
            counter++;
            T cnt = counter;
            callback(cnt, total);
        }

		bool completed()const { return counter >= total; }
        double estimatedTimeToFinish() const {
            return elapsedSeconds() * (double(total) / counter - 1);
        }
    };
}
#endif //MIYUKI_PROGRESS_H
