//
// Created by Shiina Miyuki on 2019/1/22.
//

#ifndef MIYUKI_BDPT_H
#define MIYUKI_BDPT_H

#include "../core/integrator.h"
#include "../core/geometry.h"
#include "../core/sampler.h"

namespace Miyuki {
    class BDPT : public Integrator {
    public:
        struct LightVertex {
            Vec3f hitpoint, normal, radiance;
            Float pdf, revPdf;
        };

        class Path : public std::vector<LightVertex> {

        };

    protected:
        void generateLightPath(Sampler &, Scene *, Path &, unsigned int maxS);

        void generateEyePath(Sampler &, Scene *, Path &, unsigned int maxT);

    public:
        void render(Scene *) override;
    };
}

#endif //MIYUKI_BDPT_H
