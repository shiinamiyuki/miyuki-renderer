//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_MIYUKI_H
#define MIYUKI_MIYUKI_H

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
#include <type_traits>
#include <deque>
#include <condition_variable>
#include <boost/filesystem.hpp>

#include <embree3/rtcore.h>

#include "thirdparty/fmt/format.h"
#include "thirdparty/tiny_obj_loader/tiny_obj_loader.h"
#include "thirdparty/rand48/erand48.h"
#include "thirdparty/lodepng/lodepng.h"
#include "thirdparty/tiny_jpeg.h"

namespace Miyuki {
    using Float = float;

    class NotImplemented : public std::logic_error {
    public:
        NotImplemented() : std::logic_error("Function not yet implemented") {};
    };

    inline void Assert(bool expr) {
        if (!expr) {
            throw std::runtime_error("Assertion failed");
        }
    }

    template<typename T>
    T clamp(T x, T a, T b) {
        return std::min(std::max(a, x), b);
    }

    namespace cxx = boost;
#define CHECK(expr) do{if(!(expr)){fmt::print(stderr, "{}:{} {} failed\n",__FILE__, __LINE__, #expr);}}while(0)

    void Init();

    void Exit();
    const Float PI = 3.1415926535f;
    const Float PI2 = PI / 2.0f;
    const Float PI4 = PI / 4.0f;
    const Float INVPI = 1.0f / PI;
}
#define MIYUKI_ON_WINDOWS
#endif //MIYUKI_MIYUKI_H
