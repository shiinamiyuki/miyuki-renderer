//
// Created by Shiina Miyuki on 2019/2/3.
//

#include "../core/memory.h"
#include "../core/util.h"

#define BOOST_AUTO_TEST_MAIN
#define  BOOST_TEST_MODULE TestMemory

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/filesystem/fstream.hpp>

namespace utf = boost::unit_test;
using namespace Miyuki;

BOOST_AUTO_TEST_CASE(Benchmark) {
    MemoryArena arena;
    const int N = 1000000;
    for (size_t s = 16; s <= 1024; s *= 2) {
        auto t = runtime([&]() {
            for (int i = 0; i < N; i++) {
                arena.alloc(s);
            }
        });
        arena.reset();
        fmt::print("{}bytes {}M\n", s, N / t / 1e6);
    }
}

struct MemoryRegion {
    uint8_t *head;
    size_t size;

    bool noOverlap(const MemoryRegion &rhs) const {
        return rhs.head >= head + size || head >= rhs.head + rhs.size;
    }

    void write() const {
        for (size_t i = 0; i < size; i++) {
            head[i] = 0xcc;
        }

    }
};

BOOST_AUTO_TEST_CASE(Big) {
    MemoryArena arena;
    const int N = 100;
    size_t s = 1024 * 1024;
    {
        std::vector<MemoryRegion> regions;
        for (int i = 0; i < N; i++) {
            MemoryRegion region;
            region.head = arena.alloc<uint8_t>(s);
            region.size = s;
            region.write();
            regions.emplace_back(region);
        }

        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                if (!regions[i].noOverlap(regions[j])) {
                    BOOST_ERROR("Ouch...");
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(TestOverlapSmall) {
    MemoryArena arena;
    const int N = 100;
    size_t s = 4;
    {
        std::vector<MemoryRegion> regions;
        for (int i = 0; i < N; i++) {
            MemoryRegion region;
            region.head = arena.alloc<uint8_t>(s);
            region.size = s;
            region.write();
            regions.emplace_back(region);
        }

        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                if (!regions[i].noOverlap(regions[j])) {
                    BOOST_ERROR("Ouch...");
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(TestOverlap) {
    MemoryArena arena;
    const int N = 100;
    for (size_t s = 16; s <= 1024; s *= 2) {
        std::vector<MemoryRegion> regions;
        for (int i = 0; i < N; i++) {
            MemoryRegion region;
            region.head = arena.alloc<uint8_t>(s);
            region.size = s;
            region.write();
            regions.emplace_back(region);
        }

        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                if (!regions[i].noOverlap(regions[j])) {
                    BOOST_ERROR("Ouch...");
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(TestOverlapReset) {
    MemoryArena arena;
    const int N = 100;
    for (size_t s = 16; s <= 1024; s *= 2) {
        std::vector<MemoryRegion> regions;
        for (int i = 0; i < N; i++) {
            MemoryRegion region;
            region.head = arena.alloc<uint8_t>(s);
            region.size = s;
            region.write();
            regions.emplace_back(region);
        }

        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                if (!regions[i].noOverlap(regions[j])) {
                    BOOST_ERROR("Ouch...");
                }
            }
        }
        arena.reset();
    }
}

