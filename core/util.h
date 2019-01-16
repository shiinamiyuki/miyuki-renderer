//
// Created by xiaoc on 2019/1/8.
//

#ifndef MIYUKI_UTIL_HPP
#define MIYUKI_UTIL_HPP

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <thread>
#include <chrono>
#include <ctime>
#include <xmmintrin.h>
#include <immintrin.h>
#include <algorithm>
#include <functional>
#include <random>
#include <limits>
#include <mutex>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <boost/filesystem.hpp>

#include <embree3/rtcore.h>

#include "../lib/fmt/format.h"
#include "../lib/tiny_obj_loader/tiny_obj_loader.h"
#include "../lib/rand48/erand48.h"
#include "../lib/lodepng/lodepng.h"

namespace Miyuki {
    using Float = float;

    template<typename Iter, typename Func>
    inline void parallelForEach(Iter begin, Iter end, Func f) {
#pragma omp parallel for
        for (Iter i = begin; i < end; i++) {
            f(i);
        }
    }

    template<typename Iter, typename Func>
    inline void forEach(Iter begin, Iter end, Func f) {
        for (Iter i = begin; i < end; i++) {
            f(i);
        }
    }

    template<typename Func>
    inline void parallelFor(size_t begin, size_t end, Func f) {
        parallelForEach(begin, end, f);
    }

    template<typename T>
    T clamp(T x, T a, T b) {
        if (x < a)
            return a;
        if (x > b)
            return b;
        return x;
    }

    RTCDevice GetEmbreeDevice();

    void Init();

    void Exit();
    namespace cxx = boost;
}
#define let const auto
#endif //MIYUKI_UTIL_HPP
