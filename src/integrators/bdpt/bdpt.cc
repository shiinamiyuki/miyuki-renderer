//
// Created by Shiina Miyuki on 2019/3/10.
//

#include "bdpt.h"

namespace Miyuki {

    Spectrum BDPT::L(RenderContext &ctx, Scene &scene) {
        ctx.sampler->startDimension(0);
        auto lightSubPath = generateLightSubPath(scene, ctx, minDepth, maxDepth);
        int dim = 4 + 4 * maxDepth;
        ctx.sampler->startDimension(dim);
        auto cameraSubPath = generateCameraSubPath(scene, ctx, minDepth + 1, maxDepth + 1);
        dim += 4 + 4 * maxDepth;
        ctx.sampler->startDimension(dim);
        for (int i = 0; i < lightSubPath.N; i++)
            CHECK(!std::isnan(lightSubPath[i].pdfFwd) && !std::isnan(lightSubPath[i].pdfRev));
        for (int i = 0; i < cameraSubPath.N; i++)
            CHECK(!std::isnan(cameraSubPath[i].pdfFwd) && !std::isnan(cameraSubPath[i].pdfRev));
        auto &film = *scene.film;
        Spectrum LPath;
        for (int t = 1; t < cameraSubPath.N; t++) {
            for (int s = 0; s < lightSubPath.N; s++) {
                int depth = t + s - 2;
                if ((s == 1 && t == 1) || depth < 0 || depth > maxDepth)
                    continue;
                Point2i raster;
                auto LConnect = connectBDPT(scene, ctx, lightSubPath, cameraSubPath, s, t, &raster, nullptr);
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
                      int s, int t, Point2i *raster, Float *weight) {
        // TODO: infinite light
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
                    Li = L.beta * L.f(sampled, Bidir::TransportMode::importance) * sampled.beta;
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
                Li *= E.beta * E.f(sampled, Bidir::TransportMode::radiance) / pdf * Vec3f::absDot(wi, E.Ns) /
                      scene.pdfLightChoice(light);
                if (Li.isBlack())return {};
                if (!tester.visible(scene))return {};
                // TODO:
                // sampled.pdfFwd =
            } else {
                if (L.connectable() && E.connectable()) {
                    Li = L.beta * E.beta * L.f(E, Bidir::TransportMode::importance) *
                         E.f(L, Bidir::TransportMode::radiance);
                    if (Li.isBlack())return Li;
                    Li *= Bidir::G(L, E);
                    VisibilityTester tester;
                    tester.shadowRay = Ray{L.ref, (E.ref - L.ref).normalized()};
                    Assert(E.event);
                    tester.primId = E.event->getIntersection()->primId;
                    tester.geomId = E.event->getIntersection()->geomId;
                    if (!tester.visible(scene))return {};
                }
            }
        }
        if (Li.isBlack())return {};
        if (s + t == 2)return Li;
        Float naiveWeight = 1.0f / (s + t);
        Li *= naiveWeight;
        return Li;
    }

    BDPT::BDPT(const ParameterSet &set) {
        minDepth = set.findInt("bdpt.minDepth", 3);
        maxDepth = set.findInt("bdpt.maxDepth", 5);
        spp = set.findInt("bdpt.spp", 4);
        maxRayIntensity = set.findFloat("bdpt.maxRayIntensity", 10000.0f);
    }

    Bidir::SubPath BDPT::generateLightSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth) {
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
        auto path = Bidir::RandomWalk(vertices + 1, ray, beta, pdfDir, scene, ctx, minDepth, maxDepth,
                                      Bidir::TransportMode::radiance);
        return Bidir::SubPath(vertices, 1 + path.N);
    }

    Bidir::SubPath BDPT::generateCameraSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth) {
        auto vertices = ctx.arena->alloc<Bidir::Vertex>(size_t(maxDepth + 1));
        Spectrum beta(1, 1, 1);
        vertices[0] = Bidir::CreateCameraVertex(ctx.camera, ctx.raster, ctx.primary, 1.0f, beta);
        auto path = Bidir::RandomWalk(vertices + 1, ctx.primary, beta,
                                      1.0f, scene, ctx, minDepth, maxDepth,
                                      Bidir::TransportMode::importance);
        return Bidir::SubPath(vertices, 1 + path.N);
    }
}