//
// Created by xiaoc on 2019/1/8.
//

#ifndef MIYUKI_BENCH_TEST_HPP
#define MIYUKI_BENCH_TEST_HPP

#include "core/util.h"

void bench_time(std::function<void(void)> f) {
    auto start = std::chrono::system_clock::now();
    f();
    auto end = std::chrono::system_clock::now();
    auto elapsed_seconds = end - start;
    fmt::print("{}s\n", elapsed_seconds.count() / 1e9);
}

void sum_array(size_t N, float *A, float *B, float *C) {
    Miyuki::parallel_for(0, N, [&](auto i) {
        C[i] = A[i] + B[i];
    });
};

void sum_array2(size_t N, float *A, float *B, float *C) {
    Miyuki::for_each<size_t>(0u, N, [&](auto i) {
        C[i] = A[i] + B[i];
    });
};

void sum_array3(size_t N, float *A, float *B, float *C) {
    size_t i;
    for (i = 0; i + 8 < N; i += 8) {
        C[i] = A[i] + B[i];
        C[i + 1] = A[i + 1] + B[i + 1];
        C[i + 2] = A[i + 2] + B[i + 2];
        C[i + 3] = A[i + 3] + B[i + 3];
        C[i + 4] = A[i + 4] + B[i + 4];
        C[i + 5] = A[i + 5] + B[i + 5];
        C[i + 6] = A[i + 6] + B[i + 6];
        C[i + 7] = A[i + 7] + B[i + 7];
    }
    while (i < N) {
        C[i] = A[i] + B[i];
        i++;
    }
};
#endif //MIYUKI_BENCH_TEST_HPP
