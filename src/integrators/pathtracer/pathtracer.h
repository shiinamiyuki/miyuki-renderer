//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_PATH_H
#define MIYUKI_PATH_H

#include "../integrator.h"
#include "../../math/geometry.h"
#include "../../samplers/sampler.h"
#include "../../core/spectrum.h"
#include "../../core/intersection.h"

namespace Miyuki {
    struct RenderContext;

    class Light;

    class PSSMLTUnidirectional;

    class PathTracer : public Integrator {
        //void renderPixel(const Point2i &, Sampler &);
    protected:
        friend class PSSMLTUnidirectional;

        void iteration(Scene &);

        Spectrum render(const Point2i &, RenderContext &, Scene &);

    public:
        void render(Scene &) override;
    };


}
#endif //MIYUKI_PATH_H
