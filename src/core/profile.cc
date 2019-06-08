//
// Created by Shiina Miyuki on 2019/3/3.
//

#include <core/profile.h>

namespace Miyuki {

	Timer::Timer() {
        start = std::chrono::system_clock::now();
    }

    double Timer::elapsedSeconds() const {
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        return elapsed_seconds.count();
    }

	Timer::~Timer() {

    }
}