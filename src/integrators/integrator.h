//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_INTEGRATOR_H
#define MIYUKI_INTEGRATOR_H

#include "miyuki.h"
#include "core/scene.h"
#include "core/spectrum.h"
#include "core/scatteringevent.h"
#include <bidir/vertex.h>
#include "thirdparty/hilbert/hilbert_curve.hpp"
#include "rendersession.hpp"

namespace Miyuki {
    inline void HilbertMapping(const Point2i &nTiles, std::vector<Point2f> &hilbertMapping) {
        int M = std::ceil(std::log2(std::max(nTiles.x(), nTiles.y())));
        for (int i = 0; i < pow(2, M + M); i++) {
            int tx, ty;
            ::d2xy(M, i, tx, ty);
            if (tx >= nTiles.x() || ty >= nTiles.y())
                continue;
            hilbertMapping.emplace_back(tx, ty);
        }
    }


    class Scene;

    static const int TileSize = 16;

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

        virtual std::unique_ptr<RenderSession> saveSession() {return nullptr;}

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

    class DirectLightingIntegrator : public SamplerIntegrator {
    public:
    protected:
        Spectrum Li(RenderContext &ctx, Scene &scene) override {
            using Bidir::Vertex, Bidir::SubPath;
            auto vertices = ctx.arena->alloc<Bidir::Vertex>(size_t(1 + 1));
            Spectrum beta(1, 1, 1);
            vertices[0] = Bidir::CreateCameraVertex(ctx.camera, ctx.raster, ctx.primary, 1.0f, beta);
            auto path = Bidir::RandomWalk(vertices + 1, ctx.primary, beta,
                                          1.0f, scene, ctx, 1, 1,
                                          TransportMode::importance);
            Spectrum Li(0, 0, 0);
            bool specular = false;
            ctx.sampler->startDimension(4 + 4 * 1);
            for (int depth = 0; depth < path.N; depth++) {
                if (specular || depth == 0) {
                    Vec3f wo = (path[depth - 1].ref - path[depth].ref).normalized();
                    Li += path[depth].beta * path[depth].Le(wo);
                }
                Li += path[depth].beta * importanceSampleOneLight(scene, ctx, *path[depth].event);
                specular = path[depth].delta;
            }
            return Li;
        }

    public:
        DirectLightingIntegrator(int spp) {
            this->spp = spp;
            maxRayIntensity = 1000;
        }
    };

}
#endif //MIYUKI_INTEGRATOR_H
