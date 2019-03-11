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
    }

    void PathMLT::render(Scene &scene) {
        VolPath::render(scene);
    }

}