//
// Created by Shiina Miyuki on 2019/3/11.
//

#include "pathmlt.h"

namespace Miyuki {

    PathMLT::PathMLT(const ParameterSet &set) : VolPath(set) {
        minDepth = set.findInt("pathmlt.minDepth", 0);
        maxDepth = set.findInt("pathmlt.maxDepth", 5);
        spp = set.findInt("pathmlt.spp", 4);
        maxRayIntensity = set.findFloat("pathmlt.maxRayIntensity", 10000.0f);
        largeStepProbability = set.findFloat("pathmlt.largeStep", 0.3);
        luminanceSamples = set.findInt("pathmlt.luminanceSamples", 100000);
        directSamples = set.findInt("pathmlt.directSamples", 16);
        nChains = set.findInt("pathmlt.nChains", 1000);
    }

    void PathMLT::render(Scene &scene) {
        Bootstrapper bootstrapper(scene, this);
        BootstrapSample bootstrapSample;
        fmt::print("Generate bootstrap samples\n");
        bootstrapper.generateBootstrapSamples(&bootstrapSample, luminanceSamples, nChains);
        fmt::print("b = {}\n", bootstrapSample.b);
    }

    Float PathMLT::Bootstrapper::f(Seed *seed, MemoryArena *arena) {
        RandomSampler sampler(seed);
        auto ctx = scene.getRenderContext(sampler.get2D(), arena, &sampler);
        return integrator->LRandomWalk(ctx, scene).luminance();
    }
}