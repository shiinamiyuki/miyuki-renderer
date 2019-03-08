//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_STATS_H
#define MIYUKI_STATS_H

#include "miyuki.h"

namespace Miyuki {
    template<typename T>
    class Statistics {

    };

    template<>
    class Statistics<int32_t> {
        std::atomic<int32_t> counter;
    public:
        Statistics() : counter(0) {}

        void update(int32_t value) { counter += value; }
    };
}
#endif //MIYUKI_STATS_H
