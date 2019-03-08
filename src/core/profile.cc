//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "profile.h"

namespace Miyuki {

    Profiler::Profiler() {
        start = std::chrono::system_clock::now();
    }

    double Profiler::elapsedSeconds() const {
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        return elapsed_seconds.count();
    }

    Profiler::~Profiler() {

    }
}