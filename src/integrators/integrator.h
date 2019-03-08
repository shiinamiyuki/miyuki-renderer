//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_INTEGRATOR_H
#define MIYUKI_INTEGRATOR_H

#include "miyuki.h"
#include "core/scene.h"
#include "core/spectrum.h"
#include "core/scatteringevent.h"

namespace Miyuki {
    class Scene;

    class Integrator {
    protected:
        virtual Spectrum L(RenderContext &ctx, Scene &) = 0;

        void makeScatteringEvent(ScatteringEvent *, RenderContext &ctx, Intersection *);

        // Importance sample one light according to its power
        // using multiple importance sampling
        Spectrum importanceSampleOneLight(Scene &scene, RenderContext &ctx, const ScatteringEvent &event);

    public:
        virtual void render(Scene &) = 0;

        virtual ~Integrator() {}
    };
}
#endif //MIYUKI_INTEGRATOR_H
