//
// Created by Shiina Miyuki on 2019/3/28.
//

#ifndef MIYUKI_GUIDED_PATH_H
#define MIYUKI_GUIDED_PATH_H

#include <integrators/volpath/volpath.h>

namespace Miyuki {
    class GuidedPath : public SamplerIntegrator {
    protected:
        Spectrum Li(RenderContext &ctx, Scene &scene) override;
    };
}
#endif //MIYUKI_GUIDED_PATH_H
