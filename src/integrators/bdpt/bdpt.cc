//
// Created by Shiina Miyuki on 2019/3/10.
//

#include "bdpt.h"

namespace Miyuki {

    Spectrum BDPT::Li(RenderContext &ctx, Scene &scene) {
        ctx.sampler->startDimension(0);
        auto lightSubPath = generateLightSubPath(scene, ctx, minDepth, maxDepth);
        // 1 to pick light, 2 to sample pos, 2 to sample dir
        // for each iteration, 2 to sample bsdf, 1 to do r.r.
        int dim = 5 + 3 * maxDepth;
        ctx.sampler->startDimension(dim);
        auto cameraSubPath = generateCameraSubPath(scene, ctx, minDepth + 1, maxDepth + 1);
        // 2 to sample lens, 2 for pixel filter
        dim += 4 + 3 * maxDepth;
        ctx.sampler->startDimension(dim);
        for (int i = 0; i < lightSubPath.N; i++) {
            CHECK(!std::isnan(lightSubPath[i].pdfFwd) && !std::isnan(lightSubPath[i].pdfRev));
        }
        for (int i = 0; i < cameraSubPath.N; i++) {
            CHECK(!std::isnan(cameraSubPath[i].pdfFwd) && !std::isnan(cameraSubPath[i].pdfRev));
        }
        auto &film = *scene.film;
        Spectrum LPath;
        for (int t = 1; t <= cameraSubPath.N; t++) {
            for (int s = 0; s <= lightSubPath.N; s++) {
                int depth = t + s - 2;
                if ((s == 1 && t == 1) || depth < 0 || depth > maxDepth)
                    continue;
                ctx.sampler->startDimension(dim + (depth) * (depth + 1) + 2 * s);
                Point2i raster;
                auto LConnect = connectBDPT(scene, ctx, lightSubPath, cameraSubPath, s, t, &raster);
                if (t != 1)
                    LPath += LConnect;
                else if (!LConnect.isBlack()) {
                    film.addSplat(raster, LConnect);
                }

            }
        }
        return LPath;
    }

    Spectrum
    BDPT::connectBDPT(Scene &scene, RenderContext &ctx,
                      Bidir::SubPath &lightSubPath, Bidir::SubPath &cameraSubPath,
                      int s, int t, Point2i *raster, bool useMIS, Float *weight) {
        if (t > 1 && s != 0 && cameraSubPath[t - 1].isInfiniteLight())
            return {};
        auto &E = cameraSubPath[t - 1];
        Bidir::Vertex sampled;
        Spectrum Li;
        if (s == 0) {
            // interpret whole path as a complete path
            auto &prev = cameraSubPath[t - 2];
            Li = E.Le(prev) * E.beta;
            //At present, the only way to sample infinite light is to sample the brdf
            if (E.isInfiniteLight())
                return Li;
        } else {
            auto &L = lightSubPath[s - 1];
            if (t == 1) {
                if (!L.connectable())return {};
                // sample a point on camera and connects it to light subpath
                VisibilityTester vis;
                Vec3f wi;
                Float pdf;
                Assert(L.event);
                Spectrum Wi = ctx.camera->sampleWi(*L.event, ctx.sampler->get2D(),
                                                   &wi, &pdf, raster, &vis);
                if (pdf > 0 && !Wi.isBlack()) {
                    // ???
                    sampled = Bidir::CreateCameraVertex(ctx.camera, *raster, vis.shadowRay, pdf, Wi / pdf);
                    Li = L.beta * L.f(sampled, TransportMode::importance) * sampled.beta;
                    if (L.onSurface()) {
                        Li *= Vec3f::absDot(wi, L.Ns);
                    }
                    if (!Li.isBlack()) {
                        if (!vis.visible(scene))return {};
                    }
                }
            } else if (s == 1) {
                // sample a point on light an connects it to camera subpath
                if (!E.connectable())return {};
                Vec3f wi;
                Float pdf;
                VisibilityTester tester;
                // dynamically sample the light source
                const auto light = L.light;
                Assert(light);
                Li = light->sampleLi(ctx.sampler->get2D(), *E.event->getIntersection(), &wi, &pdf, &tester);
                if (Li.isBlack() || pdf <= 0)return {};
                sampled = Bidir::CreateLightVertex(light, tester.shadowRay, L.Ng, pdf, Li / pdf);
                Li *= E.beta * E.f(sampled, TransportMode::radiance) / pdf * Vec3f::absDot(wi, E.Ns) /
                      scene.pdfLightChoice(light);
                if (Li.isBlack())return {};
                if (!tester.visible(scene))return {};
                sampled.pdfFwd = sampled.pdfLightOrigin(scene, E);
            } else {
                if (L.connectable() && E.connectable()) {
                    Li = L.beta * E.beta * L.f(E, TransportMode::importance) *
                         E.f(L, TransportMode::radiance);
                    if (Li.isBlack())return Li;
                    Li *= Bidir::G(L, E);
                    VisibilityTester tester;
                    tester.shadowRay = Ray{L.ref, (E.ref - L.ref).normalized()};
                    tester.shadowRay.excludePrimId = L.event->getIntersection()->primId;
                    tester.shadowRay.excludeGeomId = L.event->getIntersection()->geomId;
                    Assert(E.event);
                    tester.primId = E.event->getIntersection()->primId;
                    tester.geomId = E.event->getIntersection()->geomId;
                    if (!tester.visible(scene))return {};
                }
            }
        }
        if (Li.isBlack())return {};
        if (s + t == 2)return Li;
        if (useMIS) {
            Float mis = MISWeight(scene, ctx, lightSubPath, cameraSubPath, s, t, sampled);
            Li *= mis;
        } else {
            Li *= 1.0 / (s + t);
        }
        return Li;
    }

    Float BDPT::MISWeight(Scene &scene, RenderContext &ctx, Bidir::SubPath &lightSubPath, Bidir::SubPath &cameraSubPath,
                          int s, int t, Bidir::Vertex &sampled) {
        if (s + t == 2)
            return 1;
        // q...q_sp_t....p
        Bidir::Vertex *qs = s > 0 ? &lightSubPath[s - 1] : nullptr,
                *pt = t > 0 ? &cameraSubPath[t - 1] : nullptr,
                *qsMinus = s > 1 ? &lightSubPath[s - 2] : nullptr,
                *ptMinus = t > 1 ? &cameraSubPath[t - 2] : nullptr;
        ScopedAssignment<Bidir::Vertex> a1;
        if (s == 1) {
            a1 = {qs, sampled};
        } else if (t == 1) {
            a1 = {pt, sampled};
        }
        ScopedAssignment<bool> a2, a3;
        if (pt) {
            a2 = {&pt->delta, false};
        }
        if (qs) {
            a3 = {&qs->delta, false};
        }
        ScopedAssignment<Float> a4;
        if (pt) {
            // ptMinus  pt  qs   qsMinus
            //        ->      <-
            //            <-pt->pdfRev
            a4 = {&pt->pdfRev, s > 0 ?
                               qs->pdf(scene, qsMinus, *pt)
                                     : pt->pdfLightOrigin(scene, *ptMinus)};
        }

        ScopedAssignment<Float> a5;
        if (ptMinus) {
            a5 = {&ptMinus->pdfRev, s > 0 ? pt->pdf(scene, qs, *ptMinus) : pt->pdfLight(scene, *ptMinus)};
        }

        ScopedAssignment<Float> a6;
        if (qs) {
            a6 = {&qs->pdfRev, pt->pdf(scene, ptMinus, *qs)};
        }

        ScopedAssignment<Float> a7;
        if (qsMinus) {
            a7 = {&qsMinus->pdfRev, qs->pdf(scene, pt, *qsMinus)};
        }


        auto remap0 = [](Float x) -> Float { return x != 0 ? x: 1; };
        Float sumRi = 0;
        Float ri = 1;
        for (int i = t - 1; i > 0; i--) {
            ri *= std::abs(remap0(cameraSubPath[i].pdfRev) / remap0(cameraSubPath[i].pdfFwd));
            if (!cameraSubPath[i].delta && !cameraSubPath[i - 1].delta)
                sumRi += ri;
        }
        ri = 1;
        for (int i = s - 1; i >= 0; i--) {
            ri *= std::abs(remap0(lightSubPath[i].pdfRev) / remap0(lightSubPath[i].pdfFwd));
            bool delta = i > 0 ? lightSubPath[i - 1].delta : lightSubPath[0].light->isDeltaLight();
            if (!lightSubPath[i].delta && !delta)
                sumRi += ri;
        }
        return clamp(1.0f / (1.0f + sumRi), 0.0f, 1.0f);
    }

    BDPT::BDPT(const ParameterSet &set) {
        minDepth = set.findInt("integrator.minDepth", 3);
        maxDepth = set.findInt("integrator.maxDepth", 5);
        spp = set.findInt("integrator.spp", 4);
        maxRayIntensity = set.findFloat("integrator.maxRayIntensity", 10000.0f);
        progressive = set.findInt("integrator.progressive", false);
    }

    Bidir::SubPath BDPT::generateLightSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth) {
        if (maxDepth < 1)
            return Bidir::SubPath(nullptr, 0);
        Float pdfLightChoice;
        auto light = scene.chooseOneLight(ctx.sampler, &pdfLightChoice);
        if (!light) {
            return Bidir::SubPath(nullptr, 0);
        }
        auto vertices = ctx.arena->alloc<Bidir::Vertex>(size_t(maxDepth + 1));
        Ray ray;
        Vec3f normal;
        Float pdfPos, pdfDir;
        auto Le = light->sampleLe(ctx.sampler->get2D(), ctx.sampler->get2D(), &ray, &normal, &pdfPos, &pdfDir);
        Spectrum beta = Le * Vec3f::absDot(ray.d, normal) / (pdfLightChoice * pdfPos * pdfDir);
        *vertices = Bidir::CreateLightVertex(light, ray, normal, pdfPos, Le);
        Assert(pdfDir >= 0);
        Assert(pdfPos >= 0);
        if (Le.isBlack() || beta.isBlack() || pdfDir == 0 || pdfPos == 0)
            return Bidir::SubPath(nullptr, 0);
        auto path = Bidir::RandomWalk(vertices + 1, ray, beta, pdfDir, scene, ctx, minDepth - 1, maxDepth - 1,
                                      TransportMode::importance);
        return Bidir::SubPath(vertices, 1 + path.N);
    }

    Bidir::SubPath BDPT::generateCameraSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth) {
        Assert(maxDepth >= 1);
        auto vertices = ctx.arena->alloc<Bidir::Vertex>(size_t(maxDepth + 1));
        Spectrum beta(1, 1, 1);
        vertices[0] = Bidir::CreateCameraVertex(ctx.camera, ctx.raster, ctx.primary, 1.0f, beta);
        Float pdfPos, pdfDir;
        ctx.camera->pdfWe(ctx.primary, &pdfPos, &pdfDir);
        auto path = Bidir::RandomWalk(vertices + 1, ctx.primary, beta,
                                      pdfDir, scene, ctx, minDepth - 1, maxDepth - 1,
                                      TransportMode::radiance);
        return Bidir::SubPath(vertices, 1 + path.N);
    }

    void BDPT::render(Scene &scene) {
        fmt::print("Integrator: Bidirectional Path Tracing\n");
        for (int i = 0; i < scene.filmDimension().x(); i++) {
            for (int j = 0; j < scene.filmDimension().y(); j++) {
                scene.film->splatWeight({i, j}) = 1.0f / spp;
            }
        }
        SamplerIntegrator::render(scene);
    }


}