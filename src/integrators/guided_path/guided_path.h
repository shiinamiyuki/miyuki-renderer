//
// Created by Shiina Miyuki on 2019/3/28.
//

#ifndef MIYUKI_GUIDED_PATH_H
#define MIYUKI_GUIDED_PATH_H

#include <integrators/volpath/volpath.h>

namespace Miyuki {
    struct GuidedPathTracer;

    class GuidedPath : public SamplerIntegrator {
        friend struct GuidedPathTracer;
        Float bsdfSamplingFraction;
        int minDepth, maxDepth;
//        std::unique_ptr<GuidedPathTracer> tracer;
    protected:
        GuidedPath(const ParameterSet &set);

        Spectrum Li(RenderContext &ctx, Scene &scene) override;
    };
}
#endif //MIYUKI_GUIDED_PATH_H
