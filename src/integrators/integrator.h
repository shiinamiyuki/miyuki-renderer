//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_INTEGRATOR_H
#define MIYUKI_INTEGRATOR_H

#include "../utils/util.h"

namespace Miyuki {
    class Scene;

    struct Ray;
    struct IntersectionInfo;

    class Intersection;

    class Sampler;

    class ScatteringEvent;

    class Integrator {
    protected:
        ScatteringEvent makeScatteringEvent(const Ray &ray,IntersectionInfo *, Sampler *sampler);

    public:
        virtual ~Integrator() = default;

        virtual void render(Scene &) = 0;
    };

    template<typename T>
    Integrator *createIntegrator() { return nullptr; }
}
#endif //MIYUKI_INTEGRATOR_H
