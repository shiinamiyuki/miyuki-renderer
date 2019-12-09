// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pt.h"
#include <miyuki.renderer/camera.h>
#include <miyuki.foundation/film.h>
#include <miyuki.foundation/log.hpp>
#include <miyuki.foundation/parallel.h>
#include <miyuki.foundation/profiler.h>
#include <miyuki.renderer/sampler.h>
#include <miyuki.renderer/sampling.h>
#include <miyuki.renderer/scene.h>
#include <miyuki.renderer/lightdistribution.h>

namespace miyuki::core {

    static float MisWeight(float pdfA, float pdfB) {
        pdfA *= pdfA;
        pdfB *= pdfB;
        return pdfA / (pdfA + pdfB);
    }

    static RenderOutput PathTracerRender(const Task<RenderSettings>::ContFunc &cont,
                                         int spp, int minDepth, int maxDepth, const RenderSettings &settings,
                                         const mpsc::Sender<std::shared_ptr<Film>> &tx) {

        auto *scene = settings.scene.get();
        scene->resetRayCounter();
        Profiler profiler;
        auto filmPtr = std::make_shared<Film>(settings.filmDimension);
        auto &film = *filmPtr;
        log::log("Integrator: Pathtracer, samples: {}\n", spp);

        auto backgroundLi = [=](const Ray &ray) -> Spectrum {
            return Spectrum(0);
        };

        auto Li = [=](Sampler &sampler, Ray ray) -> Spectrum {
            Spectrum Li(0);
            Spectrum beta(1);
            bool specular = false;
            Intersection intersection;
            if (!scene->intersect(ray, intersection)) {
                return backgroundLi(ray);
            }
            int depth = 0;
            while (true) {

                if (!intersection.material)
                    break;
                BSDF *bsdf = intersection.material->bsdf.get();
                if (!bsdf)break;

                Vec3f wo = intersection.worldToLocal(normalize(-ray.d));
                ShadingPoint sp;
                sp.texCoord = intersection.shape->texCoordAt(intersection.uv);
                sp.Ng = intersection.Ng;
                sp.Ns = intersection.Ns;
                if (depth == 0 && intersection.material->emission && intersection.material->emissionStrength) {
                    Li += beta * intersection.material->emission->evaluate(sp)
                          * intersection.material->emissionStrength->evaluate(sp);
                }
                if (++depth > maxDepth) {
                    break;
                }

                // Light Sampling
                {
                    Float lightPdf = 0;
                    auto light = settings.lightDistribution->sampleLight(sampler, &lightPdf);
                    if (light) {
                        LightSample lightSample;
                        VisibilityTester visibilityTester;
                        light->sampleLi(sampler.next2D(), intersection, lightSample, visibilityTester);
                        lightPdf *= lightSample.pdf;
                        auto f = bsdf->evaluate(sp, wo, intersection.worldToLocal(lightSample.wi)) *
                                 abs(dot(lightSample.wi, intersection.Ns));
                        if (!IsBlack(f) && visibilityTester.visible(*scene)) {
                            Li += beta * f * lightSample.Li / lightPdf;
                        }
                    }
                }

                // BSDF Sampling
                {
                    BSDFSample bsdfSample;
                    bsdfSample.wo = wo;
                    bsdf->sample(sampler.next2D(), sp, bsdfSample);
                    if (std::isnan(bsdfSample.pdf) || bsdfSample.pdf <= 0.0f) {
                        break;
                    }
                    specular = (bsdfSample.sampledType & BSDF::ESpecular) != 0;

                    auto wiW = intersection.localToWorld(bsdfSample.wi);
                    beta *= bsdfSample.f * abs(dot(intersection.Ng, wiW)) / bsdfSample.pdf;
                    ray = intersection.spawnRay(wiW);
                }


                intersection = Intersection();
                if (!scene->intersect(ray, intersection)) {
                    return Li + beta * backgroundLi(ray);
                }
            }
            return Li;
        };

        ParallelFor(0, film.height, [=, &film](int64_t j, uint64_t) {
            auto sampler = settings.sampler->clone();
            for (int i = 0; i < film.width && cont(); i++) {

                sampler->startPixel(Point2i(i, j), Point2i(film.width, film.height));
                for (int s = 0; s < spp && cont(); s++) {
                    CameraSample sample;
                    sampler->startNextSample();
                    settings.camera->generateRay(sampler->next2D(), sampler->next2D(), Point2i(i, j),
                                                 Point2i(film.width, film.height), sample);
                    film.addSample(sample.pFilm, Li(*sampler, sample.ray), 1);
                }
            }
        });
        if (!cont()) {
            return {};
        }
        auto duration = profiler.elapsed<double>();
        log::log("Rendering done in {}secs, traced {} rays, {} M rays/sec\n", duration.count(), scene->getRayCounter(),
                 scene->getRayCounter() / duration.count() / 1e6f);
        tx.send(std::shared_ptr<Film>(filmPtr));
        return RenderOutput{filmPtr};
    }

    Task<RenderOutput>
    core::PathTracer::createRenderTask(const RenderSettings &settings, const mpsc::Sender<std::shared_ptr<Film>> &tx) {
        return Task<RenderOutput>([=, &tx](const Task<RenderSettings>::ContFunc &func) {
            return PathTracerRender(func, spp, minDepth, maxDepth, settings, tx);
        });
    }
}