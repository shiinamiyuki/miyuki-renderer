//
// Created by Shiina Miyuki on 2019/3/10.
//

#include "vertex.h"
#include <core/scene.h>
#include <integrators/integrator.h>

namespace Miyuki {
    namespace Bidir {

        SubPath
        RandomWalk(Vertex *vertices, Ray ray, Spectrum beta, Float pdf, Scene &scene,
                   RenderContext &ctx, int minDepth, int maxDepth,
                   TransportMode mode) {
            auto intersections = ctx.arena->alloc<Intersection>(size_t(maxDepth));
            auto events = ctx.arena->alloc<ScatteringEvent>(size_t(maxDepth));
            Float R = beta.max();
            if (R <= 0)
                return {nullptr, 0};
            return RandomWalk(vertices, intersections, events, ray, beta, pdf, scene, ctx, minDepth, maxDepth, mode,
                              [=](Spectrum _beta) -> float {
                                  return 1 - R / beta.max();
                              });
        }

        SubPath
        RandomWalk(Vertex *vertices,
                          Intersection *intersections,
                          ScatteringEvent *events,
                          Ray ray,
                          Spectrum beta,
                          Float pdf, Scene &scene, RenderContext &ctx,
                          int minDepth, int maxDepth, TransportMode mode,
                          const std::function<Float(Spectrum)> &terminationCallBack) {
            if (maxDepth == 0) { return {nullptr, 0}; }
            Float pdfFwd = pdf, pdfRev = 0;
            int depth = 0;
            Float R = beta.max();
            if (R <= 0)
                return {nullptr, 0};
            while (true) {
                auto &vertex = vertices[depth];
                auto &prev = vertices[depth - 1];
                if (!scene.intersect(ray, &intersections[depth])) {
                    if (mode == TransportMode::radiance) {
                        vertex = CreateLightVertex(scene.infiniteAreaLight.get(), ray, ray.d, pdf, beta);
                        depth++;
                    }
                    break;
                }
                // TODO: medium
                Integrator::makeScatteringEvent(&events[depth], ctx, &intersections[depth], mode);
                auto &event = events[depth];
                vertex = CreateSurfaceVertex(&events[depth], pdfFwd, beta, prev);
                if (++depth >= maxDepth) {
                    break;
                }

                auto f = event.bsdf->sample(event);
                pdfFwd = event.pdf;

                if (f.isBlack() || pdfFwd <= 0)
                    break;
                beta *= f * Vec3f::absDot(event.Ns(), event.wiW) / event.pdf;
                beta *= CorrectShadingNormal(event, event.woW, event.wiW, mode);
                ray = event.spawnRay(event.wiW);
                std::swap(event.wi, event.wo);
                std::swap(event.wiW, event.woW);
                pdfRev = event.bsdf->pdf(event);
                std::swap(event.wi, event.wo);
                std::swap(event.wiW, event.woW);
                if (event.bsdfLobe.matchFlag(BSDFLobe::specular)) {
                    vertex.delta = true;
                    pdfRev = pdfFwd = 0;
                }

                prev.pdfRev = vertex.convertDensity(pdfRev, prev);
                if (depth >= minDepth) {
                    auto p = terminationCallBack(beta);
                    if (ctx.sampler->get1D() < 1 - p) {
                        beta *= 1 - p;
                    } else {
                        break;
                    }
                }
            }
            return SubPath(vertices, depth);
        }

        Float Vertex::pdfLightOrigin(Scene &scene, const Vertex &v) const {
            // TODO: infinite lights
            // Current implementation doesn't allow rays to be emitted from infinite area lights
            if (isInfiniteLight())
                return 0;
            CHECK(light);
            auto w = (v.ref - ref).normalized();
            auto pdfChoice = scene.pdfLightChoice(light);
            Float pdfPos, pdfDir;
            light->pdfLe(Ray{ref, w}, Ng, &pdfPos, &pdfDir);
            return pdfPos * pdfChoice;
        }

    }
}