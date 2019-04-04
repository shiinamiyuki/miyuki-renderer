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
        connectBDPT(Scene &scene, RenderContext &ctx, Bidir::SubPath &lightSubPath, Bidir::SubPath &cameraSubPath,
                    int s, int t,
                    Point2f *raster, bool useMIS = true, Float *weight = nullptr);

        Bidir::SubPath generateLightSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth);

        Bidir::SubPath generateCameraSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth);

        Spectrum Li(RenderContext &ctx, Scene &scene) override;

        Float
        MISWeight(Scene &scene, RenderContext &ctx,
                  Bidir::SubPath &lightSubPath, Bidir::SubPath &cameraSubPath,
                  int s, int t, Bidir::Vertex &sampled);

        int minDepth;
        int maxDepth;
    public:
        void render(Scene &scene) override;

    public:
        BDPT(const ParameterSet &set);
    };


}
#endif //MIYUKI_BDPT_H
