//
// Created by Shiina Miyuki on 2019/3/10.
//

#ifndef MIYUKI_BDPT_H
#define MIYUKI_BDPT_H

#include "integrators/integrator.h"
#include <bidir/vertex.h>

namespace Miyuki {
    class BDPT : public SamplerIntegrator {
    protected:

        Bidir::SubPath generateLightSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth);

        Bidir::SubPath generateCameraSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth);

        Spectrum Li(RenderContext &ctx, Scene &scene) override;


        int minDepth;
        int maxDepth;
    public:
        void render(Scene &scene) override;

    public:
        BDPT(const ParameterSet &set);
    };


}
#endif //MIYUKI_BDPT_H
