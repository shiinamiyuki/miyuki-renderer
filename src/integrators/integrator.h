//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_INTEGRATOR_H
#define MIYUKI_INTEGRATOR_H

#include "miyuki.h"
#include "core/scene.h"
#include "core/spectrum.h"
#include "core/scatteringevent.h"
#include "thirdparty/hilbert/hilbert_curve.hpp"
#include "rendersession.hpp"

namespace Miyuki {
    void HilbertMapping(const Point2i &nTiles, std::vector<Point2f> &hilbertMapping);


    class Scene;

   

    class Integrator {
    protected:


        // Importance sample one light according to its power
        // using multiple importance sampling
        Spectrum importanceSampleOneLight(Scene &scene, RenderContext &ctx, const ScatteringEvent &event);

    public:
        static void makeScatteringEvent(ScatteringEvent *event,
                                        RenderContext &ctx, Intersection *isct,
                                        TransportMode mode) {
            *event = ScatteringEvent(ctx.sampler, isct, nullptr, mode);
            isct->primitive->material()->computeScatteringFunction(ctx, *event);
        }

        virtual void render(Scene &) = 0;

        virtual ~Integrator() {}

        virtual std::unique_ptr<RenderSession> saveSession() { return nullptr; }

        virtual void loadSession(const RenderSession &) {}
    };

    class SamplerIntegrator : public Integrator {
    protected:
        int spp;
        Float maxRayIntensity;
        bool progressive;

        virtual Spectrum Li(RenderContext &ctx, Scene &) = 0;

    public:
        void render(Scene &scene) override;

        virtual void renderProgressive(Scene &scene);
    };



}
#endif //MIYUKI_INTEGRATOR_H
