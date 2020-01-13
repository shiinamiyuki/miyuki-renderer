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
#include <miyuki.renderer/denoiser.h>
#include <miyuki.renderer/progressreporter.h>
#include <miyuki.foundation/arena.hpp>


namespace miyuki::core {

    static float MisWeight(float pdfA, float pdfB) {
        pdfA *= pdfA;
        pdfB *= pdfB;
        return pdfA / (pdfA + pdfB);
    }

    static RenderOutput PathTracerRender(bool denoise, const Task<RenderSettings>::ContFunc &cont,
                                         int spp, int minDepth, int maxDepth, const RenderSettings &settings,
                                         const mpsc::Sender<std::shared_ptr<Film>> &tx) {

        auto *scene = settings.scene.get();
        scene->resetRayCounter();
        Profiler profiler;
        auto filmPtr = std::make_shared<Film>(settings.filmDimension);
        auto &film = *filmPtr;
        log::log("Integrator: MIS Path Tracer, samples: {}\n", spp);

        auto backgroundLi = [=](const Ray &ray) -> Spectrum {
            return Spectrum(0);
        };

        bool enableNEE = true;
        auto Li = [=](Sampler &sampler, Ray ray) -> Spectrum {
            Spectrum Li(0);
            Spectrum beta(1);
            bool specular = false;
            Intersection intersection, prevIntersection;
            if (!scene->intersect(ray, intersection)) {
                return backgroundLi(ray);
            }
            Float prevScatteringPdf = 0.0f;
            int depth = 0;
            while (true) {

                if (!intersection.material)
                    break;
                BSDF *bsdf = intersection.material->bsdf.get();
                if (!bsdf)break;

                Vec3f wo = intersection.worldToLocal(normalize(-1.0f * ray.d));
                ShadingPoint sp;
                sp.texCoord = intersection.shape->texCoordAt(intersection.uv);
                sp.Ng = intersection.Ng;
                sp.Ns = intersection.Ns;

                if (intersection.material->emission && intersection.material->emissionStrength &&
                    dot(ray.d, intersection.Ng) < 0) {

                    auto light = intersection.shape->light;
                    auto lightPdf = settings.lightDistribution->lightPdf(light);
                    if (!enableNEE || depth == 0 || !light || lightPdf <= 0.0f || specular) {
                        Li += beta * intersection.material->emission->evaluate(sp)
                              * intersection.material->emissionStrength->evaluate(sp);
                    } else {
                        lightPdf *= light->pdfLi(prevIntersection, ray.d);
                        auto weight = MisWeight(prevScatteringPdf, lightPdf);
                        Li += beta * weight * intersection.material->emission->evaluate(sp)
                              * intersection.material->emissionStrength->evaluate(sp);
                    }
                }
                if (++depth > maxDepth) {
                    break;
                }


                BSDFSample bsdfSample;
                // BSDF Sampling
                {

                    bsdfSample.wo = wo;
                    bsdf->sample(sampler.next2D(), sp, bsdfSample);
                    MIYUKI_CHECK(!std::isnan(bsdfSample.pdf));
                    MIYUKI_CHECK(bsdfSample.pdf >= 0.0);
                    MIYUKI_CHECK(minComp(bsdfSample.f) >= 0.0f);
                    if (std::isnan(bsdfSample.pdf) || bsdfSample.pdf <= 0.0f) {
                        break;
                    }
                    prevScatteringPdf = bsdfSample.pdf;
                    specular = (bsdfSample.sampledType & BSDF::ESpecular) != 0;
                }

                // Light Sampling
                if (enableNEE) {
                    Float lightPdf = 0;
                    auto light = settings.lightDistribution->sampleLight(sampler, &lightPdf);
                    if (light) {
                        LightSample lightSample;
                        VisibilityTester visibilityTester;
                        light->sampleLi(sampler.next2D(), intersection, lightSample, visibilityTester);
                        lightPdf *= lightSample.pdf;
                        auto f = bsdf->evaluate(sp, wo, intersection.worldToLocal(lightSample.wi)) *
                                 abs(dot(lightSample.wi, intersection.Ns));

                        if (lightPdf > 0 && !IsBlack(f) && visibilityTester.visible(*scene)) {

                            if (specular) {
                                Li += beta * f * lightSample.Li / lightPdf;
                            } else {
                                auto scatteringPdf = bsdf->evaluatePdf(sp, wo,
                                                                       intersection.worldToLocal(lightSample.wi));
                                MIYUKI_CHECK(!std::isnan(scatteringPdf));
                                MIYUKI_CHECK(scatteringPdf > 0.0f);
                                auto weight = MisWeight(lightPdf, scatteringPdf);
                                Li += beta * f * lightSample.Li / lightPdf * weight;
                            }
                        }
                    }
                }

                auto wiW = intersection.localToWorld(bsdfSample.wi);
                beta *= bsdfSample.f * abs(dot(intersection.Ng, wiW)) / bsdfSample.pdf;
                ray = intersection.spawnRay(wiW);

                if (depth > minDepth) {
                    auto p = std::min(1.0f, maxComp(beta)) * 0.95;
                    if (sampler.next1D() < p) {
                        beta /= p;
                    } else {
                        break;
                    }
                }

                prevIntersection = intersection;
                intersection = Intersection();
                if (!scene->intersect(ray, intersection)) {
                    Li += beta * backgroundLi(ray);
                    break;
                }

            }
            MIYUKI_CHECK(minComp(Li) >= 0.0f);
            return RemoveNaN(clamp(Li, Vec3f(0), Vec3f(1e16f)));
        };

        const size_t tileSize = 64;
        std::vector<Bounds2i> tiles;
        for (int i = 0; i < film.width; i += tileSize) {
            for (int j = 0; j < film.height; j += tileSize) {
                tiles.push_back({Vec2i(i, j), min(Vec2i(film.width, film.height), Vec2i(i + tileSize, j + tileSize))});
            }
        }

        std::mutex _reporterMutex;
        ProgressReporter reporter(tiles.size(), [=, &_reporterMutex](size_t cur, size_t total) {
            std::unique_lock<std::mutex> lock(_reporterMutex, std::try_to_lock);
            if (lock.owns_lock()) {
                PrintProgressBar(double(cur) / total);
            }
        });
        ParallelFor(0, tiles.size(), [=, &tiles, &film, &reporter](int64_t i, uint64_t) {
            auto sampler = settings.sampler->clone();
            auto &tile = tiles[i];
            for (int y = tile.pMin.y(); cont() && y < tile.pMax.y(); y++) {
                for (int x = tile.pMin.x(); x < tile.pMax.x(); x++) {
                    sampler->startPixel(Point2i(x, y), Point2i(film.width, film.height));
                    for (int s = 0; s < spp && cont(); s++) {
                        CameraSample sample;
                        sampler->startNextSample();
                        settings.camera->generateRay(sampler->next2D(), sampler->next2D(), Point2i(x, y),
                                                     Point2i(film.width, film.height), sample);
                        film.addSample(sample.pFilm, Li(*sampler, sample.ray), 1);
                    }
                }
            }
            reporter.update();
        });
        if (!cont()) {
            return {};
        }
        auto duration = profiler.elapsed<double>();
        log::log("Rendering done in {}secs, traced {} rays, {} M rays/sec\n", duration.count(), scene->getRayCounter(),
                 scene->getRayCounter() / duration.count() / 1e6f);
        tx.send(std::shared_ptr<Film>(filmPtr));
        if (denoise) {
//            auto denoiser = std::dynamic_pointer_cast<Denoiser>(CreateObject("OIDNDenoiser"));
//            RGBAImage image(ivec2(0));
//            denoiser->denoise(film, image);
        }
        return RenderOutput{filmPtr};
    }

    Task<RenderOutput>
    core::PathTracer::createRenderTask(const RenderSettings &settings, const mpsc::Sender<std::shared_ptr<Film>> &tx) {
        return Task<RenderOutput>([=, &tx](const Task<RenderSettings>::ContFunc &func) {
            return PathTracerRender(denoise, func, spp, minDepth, maxDepth, settings, tx);
        });
    }

    static RenderOutput GuidedPathTracerRender(bool denoise, const Task<RenderSettings>::ContFunc &cont,
                                               int spp, int minDepth, int maxDepth, const RenderSettings &settings,
                                               const mpsc::Sender<std::shared_ptr<Film>> &tx) {
        auto *scene = settings.scene.get();
        scene->resetRayCounter();
        Profiler profiler;
        auto filmPtr = std::make_shared<Film>(settings.filmDimension);
        auto &film = *filmPtr;
        log::log("Integrator: Guided Path Tracer, samples: {}\n", spp);

        auto backgroundLi = [=](const Ray &ray) -> Spectrum {
            return Spectrum(0);
        };

        std::shared_ptr<STree> sTree(new STree(scene->getBoundingBox()));
        struct PathVertex {
            Vec3f wi;
            Point3f p;
            Float irradiance = 0.0f;
        };
        auto Li = [=](bool enableNEE, bool training, Arena<PathVertex> &arena, Sampler &sampler,
                      Ray ray) -> Spectrum {
            Spectrum Li(0);
            Spectrum beta(1);
            bool specular = false;
            //auto vertices = arena.allocN(maxDepth + 1);
            PathVertex vertices[12];
            Intersection intersection, prevIntersection;
            if (!scene->intersect(ray, intersection)) {
                return backgroundLi(ray);
            }
            Float prevScatteringPdf = 0.0f;
            int depth = 0;
            while (true) {

                if (!intersection.material)
                    break;
                BSDF *bsdf = intersection.material->bsdf.get();
                if (!bsdf)break;

                Vec3f wo = intersection.worldToLocal(normalize(-1.0f * ray.d));
                ShadingPoint sp;
                sp.texCoord = intersection.shape->texCoordAt(intersection.uv);
                sp.Ng = intersection.Ng;
                sp.Ns = intersection.Ns;

                if (intersection.material->emission && intersection.material->emissionStrength &&
                    dot(ray.d, intersection.Ng) < 0) {

                    auto light = intersection.shape->light;
                    auto lightPdf = settings.lightDistribution->lightPdf(light);
                    if (!enableNEE || depth == 0 || !light || lightPdf <= 0.0f || specular) {
                        Li += beta * intersection.material->emission->evaluate(sp)
                              * intersection.material->emissionStrength->evaluate(sp);
                    } else {
                        lightPdf *= light->pdfLi(prevIntersection, ray.d);
                        auto weight = MisWeight(prevScatteringPdf, lightPdf);
                        Li += beta * weight * intersection.material->emission->evaluate(sp)
                              * intersection.material->emissionStrength->evaluate(sp);
                    }
                }
                vertices[depth].irradiance = Li.luminance();
                vertices[depth].p = intersection.p;
                if (++depth > maxDepth) {
                    break;
                }


                BSDFSample bsdfSample;
                // BSDF Sampling
                {
                    bsdfSample.wo = wo;
                    auto u = sampler.next2D();
                    if (u[0] < 0.5f) {
                        u[0] *= 2.0f;
                        bsdf->sample(u, sp, bsdfSample);
                        bsdfSample.pdf *= 0.5f;
                    } else {
                        u[0] = (u[0] - 0.5f) * 2.0f;
                        auto w = sTree->sample(intersection.p, u);
                        bsdfSample.wi = intersection.worldToLocal(w);
                        bsdfSample.pdf = sTree->pdf(intersection.p, w) * 0.5f * Inv4Pi;
                        bsdfSample.f = bsdf->evaluate(sp, bsdfSample.wo, bsdfSample.wi);
                        bsdfSample.sampledType = BSDF::EAllButSpecular;
                    }
                    MIYUKI_CHECK(!std::isnan(bsdfSample.pdf));
                    MIYUKI_CHECK(bsdfSample.pdf >= 0.0);
                    MIYUKI_CHECK(minComp(bsdfSample.f) >= 0.0f);
                    if (std::isnan(bsdfSample.pdf) || bsdfSample.pdf <= 0.0f) {
                        break;
                    }
                    prevScatteringPdf = bsdfSample.pdf;
                    specular = (bsdfSample.sampledType & BSDF::ESpecular) != 0;
                }

                // Light Sampling
                if (enableNEE) {
                    Float lightPdf = 0;
                    auto light = settings.lightDistribution->sampleLight(sampler, &lightPdf);
                    if (light) {
                        LightSample lightSample;
                        VisibilityTester visibilityTester;
                        light->sampleLi(sampler.next2D(), intersection, lightSample, visibilityTester);
                        lightPdf *= lightSample.pdf;
                        auto f = bsdf->evaluate(sp, wo, intersection.worldToLocal(lightSample.wi)) *
                                 abs(dot(lightSample.wi, intersection.Ns));

                        if (lightPdf > 0 && !IsBlack(f) && visibilityTester.visible(*scene)) {

                            if (specular) {
                                Li += beta * f * lightSample.Li / lightPdf;
                            } else {
                                auto scatteringPdf = bsdf->evaluatePdf(sp, wo,
                                                                       intersection.worldToLocal(lightSample.wi));
                                MIYUKI_CHECK(!std::isnan(scatteringPdf));
                                MIYUKI_CHECK(scatteringPdf > 0.0f);
                                auto weight = MisWeight(lightPdf, scatteringPdf);
                                Li += beta * f * lightSample.Li / lightPdf * weight;
                            }
                        }
                    }
                }

                auto wiW = intersection.localToWorld(bsdfSample.wi);
                beta *= bsdfSample.f * abs(dot(intersection.Ng, wiW)) / bsdfSample.pdf;
                ray = intersection.spawnRay(wiW);
                vertices[depth - 1].wi = wiW;
                if (depth > minDepth) {
                    auto p = std::min(1.0f, maxComp(beta)) * 0.95;
                    if (sampler.next1D() < p) {
                        beta /= p;
                    } else {
                        break;
                    }
                }

                prevIntersection = intersection;
                intersection = Intersection();
                if (!scene->intersect(ray, intersection)) {
                    Li += beta * backgroundLi(ray);
                    break;
                }

            }
            if (training) {
                for (int i = 0; i < depth; i++) {
                    vertices[i].irradiance = Li.luminance() - vertices[i].irradiance;
                    sTree->deposit(vertices[i].p, vertices[i].wi, vertices[i].irradiance);
                }
            }
            MIYUKI_CHECK(minComp(Li) >= 0.0f);
            return RemoveNaN(clamp(Li, Vec3f(0), Vec3f(1e16f)));
        };

        const size_t tileSize = 64;
        std::vector<Bounds2i> tiles;
        for (int i = 0; i < film.width; i += tileSize) {
            for (int j = 0; j < film.height; j += tileSize) {
                tiles.push_back({Vec2i(i, j), min(Vec2i(film.width, film.height), Vec2i(i + tileSize, j + tileSize))});
            }
        }

        std::mutex _reporterMutex;
        ProgressReporter reporter(tiles.size(), [=, &_reporterMutex](size_t cur, size_t total) {
            std::unique_lock<std::mutex> lock(_reporterMutex, std::try_to_lock);
            if (lock.owns_lock()) {
                PrintProgressBar(double(cur) / total);
            }
        });
        uint32_t pass = 0;
        for (int i = 0; i < 6; i++) {
            ParallelFor(0, tiles.size(), [=, &tiles, &film](int64_t i, uint64_t) {
                auto sampler = settings.sampler->clone();
                sampler->startSample(pass == 0 ? 0u : (1u << pass));
                Arena<PathVertex> arena;
                auto &tile = tiles[i];
                for (int y = tile.pMin.y(); cont() && y < tile.pMax.y(); y++) {
                    for (int x = tile.pMin.x(); x < tile.pMax.x(); x++) {
                        sampler->startPixel(Point2i(x, y), Point2i(film.width, film.height));
                        for (int s = 0; s < (1u << pass) && cont(); s++) {
                            CameraSample sample;
                            sampler->startNextSample();
                            settings.camera->generateRay(sampler->next2D(), sampler->next2D(), Point2i(x, y),
                                                         Point2i(film.width, film.height), sample);

                            film.addSample(sample.pFilm, Li(true, true, arena, *sampler, sample.ray), 1);
                            arena.reset();
                        }
                    }
                }
            });
            log::log("Refining SDTree\n");
            sTree->refine(6000 * std::sqrt(1u << (int32_t) i));
            log::log("Done refining SDTree\n");
        }
        log::log("Start Rendering\n");
        {
            ParallelFor(0, tiles.size(), [=, &tiles, &film, &reporter](int64_t i, uint64_t) {
                auto sampler = settings.sampler->clone();
                Arena<PathVertex> arena;
                auto &tile = tiles[i];
                for (int y = tile.pMin.y(); cont() && y < tile.pMax.y(); y++) {
                    for (int x = tile.pMin.x(); x < tile.pMax.x(); x++) {
                        sampler->startPixel(Point2i(x, y), Point2i(film.width, film.height));
                        for (int s = 0; s < spp && cont(); s++) {
                            CameraSample sample;
                            sampler->startNextSample();
                            settings.camera->generateRay(sampler->next2D(), sampler->next2D(), Point2i(x, y),
                                                         Point2i(film.width, film.height), sample);

                            film.addSample(sample.pFilm, Li(false, false, arena, *sampler, sample.ray), 1);
                            arena.reset();
                        }
                    }
                }
                reporter.update();
            });
        }
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
    core::GuidedPathTracer::createRenderTask(const RenderSettings &settings,
                                             const mpsc::Sender<std::shared_ptr<Film>> &tx) {
        return Task<RenderOutput>([=, &tx](const Task<RenderSettings>::ContFunc &func) {
            return GuidedPathTracerRender(denoise, func, spp, minDepth, maxDepth, settings, tx);
        });
    }
}