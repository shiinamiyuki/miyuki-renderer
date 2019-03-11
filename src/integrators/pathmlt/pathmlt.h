//
// Created by Shiina Miyuki on 2019/3/11.
//

#ifndef MIYUKI_PATHMLT_H
#define MIYUKI_PATHMLT_H

#include <integrators/integrator.h>
#include <integrators/volpath/volpath.h>
#include <bidir/vertex.h>
#include <bidir/mutation.h>

namespace Miyuki {
    // Unidirectional Path Space MLT
    class PathMLT : public VolPath {
    protected:
        Float largeStepProbability;
        int luminanceSamples;
        int directSamples;
    public:
        PathMLT(const ParameterSet &set);

        void render(Scene &scene) override;

    protected:
    };
}
#endif //MIYUKI_PATHMLT_H
