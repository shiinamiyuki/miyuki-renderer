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

    static const int TileSize = 16;

    class Integrator {
    protected:
        virtual Spectrum L(RenderContext &ctx, Scene &) = 0;


        // Importance sample one light according to its power
        // using multiple importance sampling
        Spectrum importanceSampleOneLight(Scene &scene, RenderContext &ctx, const ScatteringEvent &event);

    public:
        static void makeScatteringEvent(ScatteringEvent *, RenderContext &ctx, Intersection *);

        virtual void render(Scene &) = 0;

        virtual ~Integrator() {}
    };

    class SamplerIntegrator : public Integrator {
    protected:
        int spp;
        Float maxRayIntensity;
    public:
        void render(Scene &scene) override;
    };
    class DirectLightingIntegrator : public SamplerIntegrator{
    public:
    protected:
        Spectrum L(RenderContext &ctx, Scene &scene) override;

    public:
        void render(Scene &scene) override;

    public:
        DirectLightingIntegrator(int spp){
            this->spp = spp;
            maxRayIntensity = 1000;
        }
    };
}
#endif //MIYUKI_INTEGRATOR_H
