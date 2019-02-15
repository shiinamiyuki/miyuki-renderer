//
// Created by Shiina Miyuki on 2019/1/31.
//

#include "stratified.h"

using namespace Miyuki;

Miyuki::StratifiedSampler::StratifiedSampler(Miyuki::Seed *s) : Sampler(s) {
    N = 8;
    delta = 1.0f / N;
    flag = true;
    streamIdx = 0;
}

Miyuki::Float Miyuki::StratifiedSampler::nextFloat() {
    return nextFloat(this->seed);
}

int32_t Miyuki::StratifiedSampler::nextInt() {
    return nrand48(seed->getPtr());
}

Point2f StratifiedSampler::nextFloat2D() {
    while (streamIdx >= coord.size()) {
        coord.emplace_back(randInt() % N, randInt() % N);
    }
    auto x = coord[streamIdx].x();
    auto y = coord[streamIdx].y();
    Float a = x * delta + nextFloat() * delta;
    Float b = y * delta + nextFloat() * delta;
    updateXY();
    streamIdx++;
    return {a, b};
}


Miyuki::Float Miyuki::StratifiedSampler::nextFloat(Miyuki::Seed *seed) {
    return erand48(seed->getPtr());
}

int32_t Miyuki::StratifiedSampler::nextInt(Miyuki::Seed *seed) {
    return nrand48(seed->getPtr());
}

void StratifiedSampler::updateXY() {
    auto &x = coord[streamIdx].x();
    auto &y = coord[streamIdx].y();
    x++;
    if (x >= N) {
        x = 0;
        y++;
    }
    if (y >= N)y = 0;
}

void StratifiedSampler::start() {
    streamIdx = 0;
}


