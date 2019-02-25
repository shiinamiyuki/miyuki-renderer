//
// Created by Shiina Miyuki on 2019/2/9.
//
#include "integrator.h"
#include "../core/scatteringevent.h"
#include "../core/scene.h"

using namespace Miyuki;

ScatteringEvent
Integrator::makeScatteringEvent(const Ray &ray, IntersectionInfo *info, Sampler *sampler) {
    return ScatteringEvent(info, sampler);
}

Spectrum Integrator::importanceSampleOneLight(Scene &scene,
                                              RenderContext &ctx,
                                              ScatteringEvent &event,
                                              bool specular) {
    Spectrum Ld;
    Float pdfLightChoice;
    auto light = scene.chooseOneLight(*ctx.sampler, &pdfLightChoice);
    ScatteringEvent scatteringEvent = event;
    BSDFType bsdfFlags = specular ? BSDFType::all :
                         BSDFType::allButSpecular;
    auto bsdf = event.getIntersectionInfo()->bsdf;
    {
        Vec3f wi;
        VisibilityTester tester;
        Float lightPdf = 0, scatteringPdf = 0;
        auto Li = light->sampleLi(ctx.sampler->nextFloat2D(), *event.getIntersectionInfo(), &wi, &lightPdf, &tester);
        scatteringEvent.wiW = wi;
        scatteringEvent.wi = scatteringEvent.worldToLocal(scatteringEvent.wiW);
        lightPdf *= pdfLightChoice;
        // sample light source
        if (lightPdf > 0 && !Li.isBlack()) {
            Spectrum f;
            f = bsdf->eval(scatteringEvent) * Vec3f::absDot(wi, event.Ns);
            scatteringPdf = bsdf->pdf(event.wo, scatteringEvent.wi, BSDFType::all);
            if (!f.isBlack() && tester.visible(scene)) {
                Float weight = powerHeuristic(1, lightPdf, 1, scatteringPdf);
                Ld += f * Li * weight / lightPdf;
            }
        }
    }
    scatteringEvent.u = ctx.sampler->nextFloat2D();

    /* This part is different from pbrt
     * Instead of sample contribution of the previous light source,
     * we randomly sample a ray according to BSDF and if it hit a light source,
     * we compute the reverse probability of sampling that light source (similar to MIS in bdpt)
     * */
    {
        Spectrum f;
        bool sampledSpecular;
        f = bsdf->sample(scatteringEvent);
        Float scatteringPdf = scatteringEvent.pdf;
        Vec3f wi = scatteringEvent.wiW;
        Float lightPdf = 0;
        f *= Vec3f::absDot(wi, scatteringEvent.Ns);
        sampledSpecular = ((int) event.sampledType & (int) BSDFType::specular) != 0;
        if (!f.isBlack() && scatteringPdf > 0) {
            if (!sampledSpecular) {
                Ray ray = event.spawnRay(wi);
                IntersectionInfo lightIntersect;
                // hit light source
                if (scene.intersect(ray, &lightIntersect) && lightIntersect.primitive->light) {
                    light = lightIntersect.primitive->light;
                    pdfLightChoice = scene.pdfLightChoice(light);
                    lightPdf = light->pdfLi(*event.getIntersectionInfo(), wi) * pdfLightChoice;
                    if (lightPdf > 0) {
                        Float weight = powerHeuristic(1, scatteringPdf, 1, lightPdf);
                        Spectrum Li = lightIntersect.Le(-1 * wi);
                        Ld += f * Li * weight / scatteringPdf;
                    }
                }
            }
        }
    }
    return Ld;
}
