//
// Created by Shiina Miyuki on 2019/3/28.
//

#ifndef MIYUKI_PATHTRACER_HPP
#define MIYUKI_PATHTRACER_HPP

#include <integrators/integrator.h>

// WIP
// Generic Path Tracer using CRTP to improve efficiency (maybe)
namespace Miyuki {
    template<typename IntegratorType>
    struct PathTracer {
        IntegratorType *getPointer() {
            return static_cast<IntegratorType *>(this);
        }

        Spectrum sample(ScatteringEvent &event) {
            return getPointer()->sampleEvent(event);
        }

        Float pdf(ScatteringEvent &event) {
            return getPointer()->pdfEvent(event);
        }

        Spectrum estimateDirect(Scene &scene, RenderContext &ctx, const ScatteringEvent &event) {
            return getPointer()->nextEventEstimation(scene, ctx, event);
        }

        Spectrum sampleOneLightMIS(Scene &scene,
                                   const std::vector<std::shared_ptr<Light>> &lights,
                                   const Distribution1D &lightDist,
                                   RenderContext &ctx, const ScatteringEvent &event) {
            Spectrum Ld(0, 0, 0);
            Float pdfLightChoice;
            Point2f lightSample = ctx.sampler->get2D();
            Point2f bsdfSample = ctx.sampler->get2D();
            auto idx = lightDist.sampleInt(ctx.sampler->get1D());
            auto light = lights[idx].get();
            pdfLightChoice = lightDist.pdf(idx);
            if (!light) {
                return {};
            }
            auto bsdf = event.bsdf;
            ScatteringEvent scatteringEvent = event;
            // sample light source
            {
                Vec3f wi;
                Float lightPdf;
                VisibilityTester tester;
                auto Li = light->sampleLi(lightSample, *event.getIntersection(), &wi, &lightPdf, &tester);
                lightPdf *= pdfLightChoice;

                if (lightPdf > 0 && !Li.isBlack()) {
                    scatteringEvent.wiW = wi;
                    scatteringEvent.wi = scatteringEvent.worldToLocal(wi);
                    Spectrum f = bsdf->f(scatteringEvent) * Vec3f::absDot(wi, event.Ns());

                    Float scatteringPdf = pdf(scatteringEvent);
                    if (!f.isBlack() && tester.visible(scene)) {
                        Float weight = PowerHeuristics(lightPdf, scatteringPdf);
                        Ld += f * Li * weight / lightPdf;
                    }
                }
            }
            // sample bsdf
            {
                scatteringEvent.u = bsdfSample;
                Spectrum f = sample(scatteringEvent);
                const auto wi = scatteringEvent.wiW;
                f *= Vec3f::absDot(scatteringEvent.Ns(), wi);
                Float scatteringPdf = scatteringEvent.pdf;
                bool sampledSpecular = scatteringEvent.bsdfLobe.matchFlag(BSDFLobe::specular);
                if (!f.isBlack() && scatteringPdf > 0 && !sampledSpecular) {
                    Ray ray = scatteringEvent.spawnRay(wi);
                    Intersection isct;
                    if (scene.intersect(ray, &isct) && isct.primitive->light()) {
                        light = isct.primitive->light();
                        Float lightPdf = light->pdfLi(*event.getIntersection(), wi) * scene.pdfLightChoice(light);
                        auto Li = isct.Le(-1 * wi);
                        if (lightPdf > 0) {
                            Float weight = PowerHeuristics(scatteringPdf, lightPdf);
                            Ld += f * Li * weight / scatteringPdf;
                        }
                    }
                }
            }
            return Ld;
        }

        Spectrum Li(Scene &scene,
                    RayDifferential ray,
                    RenderContext &ctx,
                    int minDepth,
                    int maxDepth) {
            int depth = 0;
            ScatteringEvent event;
            Intersection intersection;
            Spectrum Li(0), beta(1);
            bool specular = false;
            while (true) {
                if (!scene.intersect(ray, &intersection)) {
                    Li += beta * scene.infiniteAreaLight->L(ray);
                    break;
                }
                Integrator::makeScatteringEvent(&event, ctx, &intersection, TransportMode::radiance);
                if (specular || depth == 0) {
                    Li += event.Le(-1 * ray.d) * beta;
                }
                if (++depth >= maxDepth) {
                    break;
                }

                Li += beta * estimateDirect(scene, ctx, event);

                auto f = sample(event);
                if (event.pdf <= 0 || f.isBlack()) {
                    break;
                }
                specular = event.bsdfLobe.matchFlag(BSDFLobe::specular);
                ray = event.spawnRay(event.wiW);
                beta *= f * Vec3f::absDot(event.wiW, event.Ns()) / event.pdf;
                Float p = beta.max();
                if (depth >= minDepth) {
                    auto u = ctx.sampler->get1D();
                    if (p < 1) {
                        if (u < p) {
                            beta /= p;
                        } else {
                            break;
                        }
                    }
                }
            }
            return Li;
        }
    };
}
#endif //MIYUKI_PATHTRACER_HPP
