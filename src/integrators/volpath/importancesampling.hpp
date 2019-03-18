//
// Created by Shiina Miyuki on 2019/3/17.
//

#ifndef MIYUKI_IMPORTANCESAMPLING_HPP
#define MIYUKI_IMPORTANCESAMPLING_HPP

#include <core/scatteringevent.h>
#include <math/func.h>
#include <lights/light.h>
#include <math/distribution.h>
#include <core/rendercontext.h>
#include <core/scene.h>

namespace Miyuki {
    struct ImportanceLightSampler {
        const std::vector<std::shared_ptr<Light>> &lights;
        Scene &scene;

        ImportanceLightSampler(Scene &scene, const std::vector<std::shared_ptr<Light>> &lights)
                : lights(lights), scene(scene) {}

        Spectrum estimateDirectLighting(
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

                    Float scatteringPdf = bsdf->pdf(scatteringEvent);
                    if (!f.isBlack() && tester.visible(scene)) {
                        Float weight = PowerHeuristics(lightPdf, scatteringPdf);
                        Ld += f * Li * weight / lightPdf;
                    }
                }
            }
            // sample brdf
            {
                scatteringEvent.u = bsdfSample;
                Spectrum f = bsdf->sample(scatteringEvent);
                const auto wi = scatteringEvent.wiW;
                f *= Vec3f::absDot(scatteringEvent.Ns(), wi);
                Float scatteringPdf = scatteringEvent.pdf;
                bool sampledSpecular = scatteringEvent.bsdfLobe.matchFlag(BSDFLobe::specular);
                if (!f.isBlack() && scatteringPdf > 0 && !sampledSpecular) {
                    Ray ray = scatteringEvent.spawnRay(wi);
                    Intersection isct;
                    if (scene.intersect(ray, &isct) && isct.primitive->light) {
                        light = isct.primitive->light;
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
    };
}
#endif //MIYUKI_IMPORTANCESAMPLING_HPP
