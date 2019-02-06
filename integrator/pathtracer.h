//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_PATH_H
#define MIYUKI_PATH_H

#include "../core/integrator.h"
#include "../core/geometry.h"
#include "../core/sampler.h"
#include "../core/spectrum.h"
#include "../core/interaction.h"

namespace Miyuki {
    struct RenderContext;
    class PSSMLTUnidirectional;
    class PathTracer : public Integrator {
        //void renderPixel(const Point2i &, Sampler &);
    protected:
        friend class PSSMLTUnidirectional;
        void iteration(Scene &);

        Spectrum render(const Point2i &, RenderContext &, Scene &);

        Spectrum importanceSampleOneLight(const Interaction &,
                Scene &,
                RenderContext &ctx,
                bool specular=false);

    public:
        void render(Scene &) override;
    };


}
#endif //MIYUKI_PATH_H
