//
// Created by Shiina Miyuki on 2019/3/28.
//

#ifndef MIYUKI_PATHTRACER_HPP
#define MIYUKI_PATHTRACER_HPP

#include <integrators/integrator.h>

// WIP
// Generic Path Tracer using CRTP to improve efficiency (maybe)
namespace Miyuki {
    template<typename IntegratorType, typename EventType>
    struct PathTracer {
        IntegratorType *getPointer() {
            return static_cast<IntegratorType *>(this);
        }

        Spectrum sample(EventType &event) {
            return getPointer()->sampleImpl(event);
        }

        Float pdf(EventType &event) {
            return getPointer()->pdfImpl(event);
        }

        /* The function has an ugly interface.
         * When we are doing MIS, bsdf is also sampled. If the sampled ray doesn't hit a light source,
         * we can use that information and skip next ray intersection
         * */
        Spectrum estimateDirect(Scene &scene,
                                RenderContext &ctx,
                                const EventType &event,
                                Spectrum &sampledF,
                                EventType *sampledEvent,
                                Intersection *intersection,
                                bool *sampleValid) {
            return getPointer()->nextEventEstimation(scene, ctx, event,
                                                     sampledF, sampledEvent, intersection,
                                                     sampleValid);
        }

        Spectrum sampleOneLightMIS(Scene &scene,
                                   const std::vector<std::shared_ptr<Light>> &lights,
                                   const Distribution1D &lightDist,
                                   RenderContext &ctx, const EventType &event,
                                   Spectrum &sampledF,
                                   EventType *sampledEvent,
                                   Intersection *intersection,
                                   bool *sampleValid) {
            Spectrum Ld(0);
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
            EventType scatteringEvent = event;
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
                    Intersection &isct = *intersection;
                    if (scene.intersect(ray, &isct)) {
                        if (isct.primitive->light()) {
                            light = isct.primitive->light();
                            Float lightPdf = light->pdfLi(*event.getIntersection(), wi) * scene.pdfLightChoice(light);
                            auto Li = isct.Le(-1 * wi);
                            if (lightPdf > 0) {
                                Float weight = PowerHeuristics(scatteringPdf, lightPdf);
                                Ld += f * Li * weight / scatteringPdf;
                            }
                        } else {
                            *sampledEvent = scatteringEvent;
                            *sampleValid = true;
                            sampledF = f;
                        }
                    }
                }
            }
            return Ld;
        }

        void makeScatteringEvent(EventType *event, RenderContext &ctx, Intersection *intersection) {
            getPointer()->makeScatteringEventImpl(event, ctx, intersection);
        }

        Spectrum Li(Scene &scene,
                    RayDifferential ray,
                    RenderContext &ctx,
                    int minDepth,
                    int maxDepth) {
            int depth = 0;
            EventType event;
            Intersection intersection;
            Intersection tempIsct;
            Spectrum Li(0), beta(1);
            bool specular = false;
            bool valid = false;
            while (true) {
                if (valid) {
                    intersection = tempIsct;
                } else if (!scene.intersect(ray, &intersection)) {
                    Li += beta * scene.infiniteAreaLight->L(ray);
                    break;
                }

                makeScatteringEvent(&event, ctx, &intersection);
                if (specular || depth == 0) {
                    Li += event.Le(-1 * ray.d) * beta;
                }
                if (++depth >= maxDepth) {
                    break;
                }
                ScatteringEvent temp;
                valid = false;
                Spectrum sampledF;

                Li += beta * estimateDirect(scene, ctx, event, sampledF, &temp, &tempIsct, &valid);

                Spectrum f;
                if (!valid) {
                    f = sample(event);
                } else {
                    event = temp;
                    f = sampledF;
                }
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
