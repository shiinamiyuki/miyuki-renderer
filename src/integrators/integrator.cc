//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "integrator.h"
#include "math/func.h"
#include "lights/light.h"

namespace Miyuki {

    void Integrator::makeScatteringEvent(ScatteringEvent *event, RenderContext &ctx, Intersection *isct) {
        *event = ScatteringEvent(ctx.sampler, isct, nullptr);
        isct->primitive->material()->computeScatteringFunction(ctx, *event);
    }

    Spectrum Integrator::importanceSampleOneLight(Scene &scene, RenderContext &ctx, const ScatteringEvent &event) {
        Spectrum Ld(0, 0, 0);
        Float pdfLightChoice;
        Point2f lightSample = ctx.sampler->get2D();
        Point2f bsdfSample = ctx.sampler->get2D();
        auto light = scene.chooseOneLight(ctx.sampler, &pdfLightChoice);
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
}