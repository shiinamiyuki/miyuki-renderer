//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_INTEGRATOR_H
#define MIYUKI_INTEGRATOR_H

#include "../utils/util.h"
#include "../core/spectrum.h"

namespace Miyuki {
    class Scene;

    struct RenderContext;
    struct Ray;
    struct IntersectionInfo;

    class Intersection;

    class Sampler;

    class ScatteringEvent;

    class Integrator {
    protected:
        ScatteringEvent makeScatteringEvent(const Ray &ray, IntersectionInfo *, Sampler *sampler);

        virtual Spectrum importanceSampleOneLight(Scene &scene,
                                                  RenderContext &ctx,
                                                  ScatteringEvent &event,
                                                  bool specular = false);

    public:
        virtual ~Integrator() = default;

        virtual void render(Scene &) = 0;
    };

    template<typename T>
    Integrator *createIntegrator() { return nullptr; }
}
#endif //MIYUKI_INTEGRATOR_H
