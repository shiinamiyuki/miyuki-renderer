//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_PATH_H
#define MIYUKI_PATH_H

#include "../core/integrator.h"
#include "../core/geometry.h"
#include "../core/sampler.h"
#include "../core/spectrum.h"

namespace Miyuki {
    struct RenderContext;

    class PathTracer : public Integrator {
        //void renderPixel(const Point2i &, Sampler &);
    protected:
        void iteration(Scene &);

        Spectrum render(const Point2i &, RenderContext &, Scene &);

    public:
        void render(Scene &) override;
    };


}
#endif //MIYUKI_PATH_H
