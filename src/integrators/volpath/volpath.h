//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_VOLPATH_H
#define MIYUKI_VOLPATH_H

#include <core/scene.h>

#include "integrators/integrator.h"
#include "core/parameter.h"

namespace Miyuki {
    class VolPath : public SamplerIntegrator {
    protected:

        int minDepth;
        int maxDepth;
        bool caustics;
        bool adaptive;
        int nLuminanceSample = 100000;
        Float maxSampleFactor = 32;
        Float requiredPValue = 0.05;
        double maxError = 0.05;
        Float heuristic;
        bool sampleDirect = true;
        bool sampleIndirect = true;
    public:
        VolPath(const ParameterSet &set);

        void render(Scene &scene) override;

        void renderAdaptive(Scene &scene);

        void renderProgressive(Scene &scene) override;

        Spectrum estimateDirect(Scene &scene,
                                RenderContext &ctx, const ScatteringEvent &event);

    protected:
        Spectrum Li(RenderContext &ctx, Scene &scene) override;
    };

}
#endif //MIYUKI_VOLPATH_H
