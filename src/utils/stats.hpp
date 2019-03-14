//
// Created by Shiina Miyuki on 2019/1/22.
//

#ifndef MIYUKI_STATS_HPP
#define MIYUKI_STATS_HPP

#include "miyuki.h"
#include <atomic>
#define DECLARE_STATS(type,name) std::atomic<type> name(0)
#define UPDATE_STATS(name, value) name += value
#define PRINT_STATS(name) fmt::print(#name"={}\n", name)
#endif //MIYUKI_STATS_HPP
