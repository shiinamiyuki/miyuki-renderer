//
// Created by Shiina Miyuki on 2019/1/22.
//

#include "../core/sampler.h"
#include "../core/geometry.h"
#include "pssmlt.h"
#include "../core/scene.h"
#include "../core/distribution.h"
#include "../core/stats.hpp"

using namespace Miyuki;
using namespace MLT;

Float MLT::PathSeedGenerator::nextFloat() {
    auto x = RandomSampler::nextFloat();
    pathSeed.emplace_back(x);
    return x;
}

Float MLT::PathSeedGenerator::nextFloat(Seed *seed) {
    auto x = RandomSampler::nextFloat(seed);
    pathSeed.emplace_back(x);
    return x;
}


Float mutate(Float x, Float u, Float v) {
    constexpr Float s1 = 1.0f / 1024.0f, s2 = 1.0f / 64.0f;
    auto dv = static_cast<Float>(s2 * exp(-log(s2 / s1) * u));
    if (v < 0.5f) {
        x += dv;
        if (x > 1)x -= 1;
    } else {
        x -= dv;
        if (x < 0)x += 1;
    }
    return x;
}

Float Miyuki::MLTSampler::nextFloat() {
    while (streamIdx >= u.size()) {
        u.emplace_back(MLT::PrimarySample(RandomSampler::nextFloat(), time));
    }
    return primarySample(streamIdx++);
}

int Miyuki::MLTSampler::nextInt() {
    return RandomSampler::nextInt();
}

Float Miyuki::MLTSampler::nextFloat(Seed *seed) {
    while (streamIdx >= u.size()) {
        u.emplace_back(MLT::PrimarySample(RandomSampler::nextFloat(seed), time));
    }
    return primarySample(streamIdx++);
}

int Miyuki::MLTSampler::nextInt(Seed *seed) {
    return RandomSampler::nextInt(seed);
}

void MLTSampler::start() {
    streamIdx = 0;
    largeStep = RandomSampler::nextFloat() < largeStepProb;
}

MLTSampler::MLTSampler(Seed *s) : RandomSampler(s) {
    I = oldI = 0;
    time = 0;
    largeStepProb = 0.3;
    largeStepTime = 0;
}

static DECLARE_STATS(long long, modifyCounter);
static DECLARE_STATS(int, acceptCounter);

Float MLTSampler::primarySample(int i) {
    if (u[i].modify < time) {
        UPDATE_STATS(modifyCounter, 1);
        if (largeStep) {
            push(i, u[i]);
            u[i].modify = time;
            u[i].value = RandomSampler::nextFloat();
        } else {
            if (u[i].modify < largeStepTime) {
                u[i].modify = largeStepTime;
                u[i].value = RandomSampler::nextFloat();
            }
            while (u[i].modify < time - 1) {
                u[i].value = mutate(u[i].value, RandomSampler::nextFloat(), RandomSampler::nextFloat());
                u[i].modify++;
            }
            push(i, u[i]);
            u[i].value = mutate(u[i].value, RandomSampler::nextFloat(), RandomSampler::nextFloat());
            u[i].modify++;
        }
    }
    return u[i].value;
}

void MLTSampler::update(const Point2i &pos, Spectrum &L) {
    auto I = luminance(L);
    Float a = std::min(Float(1), I / oldI);
    if (a < 0)a = 0;
    if(oldI ==0)a = 1;
    newSample.radiance = L;
    newSample.pos = pos;
    newSample.weight = std::max<Float>(0, (a + largeStep) / (I / b + largeStepProb));
    oldSample.weight += std::max<Float>(0, (1 - a) / (oldI / b + largeStepProb));
    if (RandomSampler::nextFloat() < a) {
        UPDATE_STATS(acceptCounter, 1);
        oldI = I;
        contributionSample = oldSample;
        oldSample = newSample;
        if (largeStep)largeStepTime = time;
        time++;
        stack.clear();
    } else {
        contributionSample = newSample;
        while (!stack.empty()) { pop(); }
    }
}

void MLTSampler::push(int i, const MLT::PrimarySample &s) {
    stack.emplace_back(std::make_pair(i, s));
}

void MLTSampler::pop() {
    auto pair = stack.back();
    stack.pop_back();
    u[pair.first] = pair.second;
}

void MLTSampler::assignSeed(const MLT::PathSeedGenerator &pathSeedGenerator) {
    u.clear();
    for (auto &i : pathSeedGenerator.pathSeed) {
        u.emplace_back(PrimarySample(i, 0));
    }
}


void PSSMLTUnidirectional::render(Scene &scene) {
    bootstrap(scene);
    fmt::print("Running MC chains\n");
    int N = scene.option.samplesPerPixel;
    for (int i = 0; i < N; i++) {
        auto t = runtime([&]() {
            mutation(scene);
        });
        PRINT_STATS(modifyCounter);
        fmt::print("Acceptance rate: {}\n", (double) acceptCounter / ((i + 1) * samples.size()));
        fmt::print("{}th iteration in {} secs, {} M Rays/sec\n", 1 + i, t, samples.size() / t / 1e6);
    }
    scene.film.scaleImageColor(1.0f / N);
}

Spectrum PSSMLTUnidirectional::trace(int id, Scene &scene, Sampler &sampler, Point2i &pos) {
    int x = (int) clamp(sampler.nextFloat() * scene.film.width(), 0, scene.film.width() - 1);
    int y = (int) clamp(sampler.nextFloat() * scene.film.height(), 0, scene.film.height() - 1);
    pos = Point2i(x, y);
    auto ctx = scene.getRenderContext(Point2i(x, y));
    ctx.sampler = &sampler;
    return PathTracer::render(Point2i(x, y), ctx, scene);
}

void PSSMLTUnidirectional::bootstrap(Scene &scene) {
    samples.clear();
    for (int i = 0; i < scene.film.height() * scene.film.width(); i++) {
        samples.emplace_back(MLTSampler(&scene.seeds[i]));
    }
    std::vector<PathSeedGenerator> pathSeeds;
    std::vector<Spectrum> L;
    std::vector<Point2i> pos;
    pathSeeds.resize(scene.option.mltLuminanceSample);
    for (int i = 0; i < pathSeeds.size(); i++) {
        pathSeeds[i].setSeed(&scene.seeds[i]);
    }
    std::vector<Float> I;
    I.resize(pathSeeds.size());
    L.resize(pathSeeds.size());
    pos.resize(pathSeeds.size());
    Float b = 0;
    fmt::print("Generating bootstrap samples\n");
    for (int i = 0; i < pathSeeds.size(); i++) {
        auto rad = trace(i, scene, pathSeeds[i], pos[i]);
        L[i] = rad;
        I[i] = luminance(rad);
        b += luminance(rad);
    }
    b /= pathSeeds.size();
    fmt::print("b = {}\n", b);
    Distribution1D distribution1D(I.data(), I.size());
    std::random_device rd;
    std::uniform_real_distribution<double> dist;
    for (auto &i:samples) {
        int idx = distribution1D.sampleInt(dist(rd));
        //   count[idx]++;
        i.assignSeed(pathSeeds[idx]);
        i.oldI = I[idx];
        i.oldSample.radiance = L[idx];
        i.oldSample.weight = 0;
        i.contributionSample.weight = 0;
        i.newSample.weight = 0;
        i.b = b;
        i.oldSample.pos = pos[idx];
    }
}

void PSSMLTUnidirectional::mutation(Scene &scene) {
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    parallelFor(0u, samples.size(), [&](unsigned int idx) {
        auto &sampler = samples[idx];
        sampler.start();
        Point2i newPos;
        Spectrum radiance;
        radiance = trace(idx, scene, sampler, newPos);
        sampler.update(newPos, radiance);
    });

    for (int i = 0; i < samples.size(); i++) {
        auto &sampler = samples[i];
        auto radiance = sampler.contributionSample.radiance;
        radiance *= sampler.contributionSample.weight;
       film.addSplat(sampler.contributionSample.pos, radiance, 0);
    }
}

