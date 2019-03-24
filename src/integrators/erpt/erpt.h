//
// Created by Shiina Miyuki on 2019/3/22.
//

#ifndef MIYUKI_ERPT_H
#define MIYUKI_ERPT_H

#include <integrators/volpath/volpath.h>
#include <integrators/pssmlt/pssmlt.h>

namespace Miyuki {
    class ERPT : public PSSMLT {
        Float e_d = 0;

        void estimateDepositionEnergy(Scene &scene);

    public:
    protected:
        Spectrum Li(RenderContext &ctx, Scene &scene) override;

        Spectrum MCMCFirstIteration(const Point2i &raster, Scene &scene, MemoryArena *arena, MLTSampler *sampler);

    public:
        ERPT(const ParameterSet &set);

        void render(Scene &scene) override;
    };
}
#endif //MIYUKI_ERPT_H
