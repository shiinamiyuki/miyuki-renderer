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
        Spectrum
        connectBDPT(Scene &scene, RenderContext &ctx,Bidir::SubPath &lightSubPath, Bidir::SubPath &cameraSubPath, int s, int t,
                Point2i *raster,Float *weight = nullptr);

        Bidir::SubPath generateLightSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth);

        Bidir::SubPath generateCameraSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth);

        Spectrum L(RenderContext &ctx, Scene &scene) override;

        int minDepth;
        int maxDepth;
    public:
        BDPT(const ParameterSet &set);
    };


}
#endif //MIYUKI_BDPT_H
