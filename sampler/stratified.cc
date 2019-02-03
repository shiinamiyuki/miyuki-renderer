//
// Created by Shiina Miyuki on 2019/1/31.
//

#include "stratified.h"

using namespace Miyuki;

Miyuki::StratifiedSampler::StratifiedSampler(Miyuki::Seed *s) : Sampler(s) {
    N = 8;
    delta = 1.0f / N;
    flag = true;
    x = randInt() % N;
    y = randInt() % N;
}

Miyuki::Float Miyuki::StratifiedSampler::nextFloat() {
    return nextFloat(this->seed);
}

int Miyuki::StratifiedSampler::nextInt() {
    return nrand48(seed->getPtr());
}

Point2f StratifiedSampler::nextFloat2D() {
    Float a = x * delta + nextFloat() * delta;
    Float b = y * delta + nextFloat() * delta;
    updateXY();
    return {a, b};
}


Miyuki::Float Miyuki::StratifiedSampler::nextFloat(Miyuki::Seed *seed) {
    return erand48(seed->getPtr());
}

int Miyuki::StratifiedSampler::nextInt(Miyuki::Seed *seed) {
    return nrand48(seed->getPtr());
}

void StratifiedSampler::updateXY() {
    x++;
    if (x >= N) {
        x = 0;
        y++;
    }
    if (y >= N)y = 0;
}


