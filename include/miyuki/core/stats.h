//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_STATS_H
#define MIYUKI_STATS_H

#include "miyuki.h"
#include <atomic>
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

#define DECLARE_STATS(type,name) std::atomic<type> name(0)
#define DECLARE_STATS_MEMBER(type,name) std::atomic<type> name
#define UPDATE_STATS(name, value) name += value
#define PRINT_STATS(name) fmt::print(#name"={}\n", name)
#endif //MIYUKI_STATS_H
