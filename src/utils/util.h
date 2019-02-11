//
// Created by Shiina Miyuki on 2019/1/8.
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
#include <type_traits>
#include <boost/filesystem.hpp>

#include <embree3/rtcore.h>

#include "../../thirdparty/fmt/format.h"
#include "../../thirdparty/tiny_obj_loader/tiny_obj_loader.h"
#include "../../thirdparty/rand48/erand48.h"
#include "../../thirdparty/lodepng/lodepng.h"
#include "../../thirdparty/tiny_jpeg.h"

namespace Miyuki {
    using Float = float;

    template<typename Func>
    double runtime(Func f) {
        auto start = std::chrono::system_clock::now();
        f();
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        return elapsed_seconds.count();
    }

    template<typename Iter, typename Func>
    inline void parallelForEach(Iter begin, Iter end, Func f) {
#pragma omp parallel for schedule(dynamic, 1)
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

    template<typename T>
    class Ref {
        using Ty = typename std::remove_reference<T>::type;
        Ty *ptr;
    public:
        explicit Ref(Ty *p) : ptr(p) {}

        Ref() : ptr(nullptr) {}

        Ty &operator*() { return *ptr; }

        Ty *operator->() { return ptr; }

        operator bool() const { return ptr; }

        const Ty &operator*() const { return *ptr; }

        const Ty *operator->() const { return ptr; }

        Ref &operator=(Ty *p) {
            ptr = p;
            return *this;
        }
        Ty* raw()const{return ptr;}
    };

    template<typename T>
    Ref<T> makeRef(T *p) { return Ref<T>(p); }

    void readUnderPath(const std::string &filename, std::function<void(const std::string &)>);

    void loadImage(const std::string &filename, std::vector<unsigned char> &data, uint32_t *w, uint32_t *h);

    int32_t editDistance(const std::string& a, const std::string& b, bool matchCase = false);

#define CHECK(expr) do{if(!expr){fmt::print(stderr, "{}:{} {} failed\n",__FILE__, __LINE__, #expr);}}while(0)
}
#define MIYUKI_ON_WINDOWS
#endif //MIYUKI_UTIL_HPP
