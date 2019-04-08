//
// Created by Shiina Miyuki on 2019/4/5.
//

#include <samplers/sobol.h>

using namespace Miyuki;

int main() {
    Seed seed = 123;
    SobolSampler sampler(&seed);
    for (int i = 0; i < 1000; i++) {
        sampler.start();
        fmt::print("{} {}\n", sampler.get1D(), sampler.get1D());
    }
}