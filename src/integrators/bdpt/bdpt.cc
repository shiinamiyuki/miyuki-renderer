//
// Created by Shiina Miyuki on 2019/1/22.
//

#include "bdpt.h"
#include "../../core/scene.h"
#include "../../core/film.h"
#include "../../math/geometry.h"
#include "../../samplers/random.h"
#include "../../lights/light.h"
#include "../../core/film.h"
#include "../../utils/stats.hpp"

//#define BDPT_DEBUG
using namespace Miyuki;

void Miyuki::BDPT::render(Scene &scene) {
    fmt::print("Rendering\n");
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int32_t N = scene.option.samplesPerPixel;
    int32_t sleepTime = scene.option.sleepTime;
    auto maxDepth = scene.option.maxDepth;
#ifdef BDPT_DEBUG
    for (int t = 1; t <= maxDepth + 2; ++t) {
        for (int s = 0; s <= maxDepth + 12; ++s) {
            debugFilms[std::make_pair(s, t)] = Film(scene.film.width(), scene.film.height());
        }
    }
#endif
    double elapsed = 0;
    for (int32_t i = 0; i < N; i++) {
        auto t = runtime([&]() {
            iteration(scene);
            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
        });
        elapsed += t;
        fmt::print("iteration {} in {} secs, elapsed {}s, remaining {}s\n",
                   1 + i, t, elapsed, (double) (elapsed * N) / (i + 1) - elapsed);
    }
#ifdef BDPT_DEBUG
    {
        for (int t = 1; t <= maxDepth + 2; ++t) {
            for (int s = 0; s <= maxDepth + 1; ++s) {
                int depth = t + s - 2;
                if ((s == 1 && t == 1) || depth < 0 || depth > maxDepth)
                    continue;
                debugFilms[std::make_pair(s, t)].writePNG(fmt::format("data/debug/l{}_s{}_t{}.png", s + t, s, t));
            }
        }
    }
#endif
}

// IMPORTANT! What we are doing here is based on the assumption that during each pass the memory allocated
// is never freed
void BDPT::iteration(Scene &scene) {
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int maxDepth = (int) scene.option.maxDepth;
    std::mutex filmMutex, mutex;
    scene.foreachPixel([&](RenderContext &ctx) {
        Spectrum L;
        auto cameraVertices = ctx.arena.alloc<Vertex>(maxDepth + 2u);
        auto lightVertices = ctx.arena.alloc<Vertex>(maxDepth + 1u);
        auto nCamera = generateCameraSubpath(scene, ctx, maxDepth + 2, cameraVertices);
        auto nLight = generateLightSubpath(scene, ctx, maxDepth + 1, lightVertices);
        for (int i = 0; i < nLight; i++)
            CHECK(!std::isnan(lightVertices[i].pdfFwd) && !std::isnan(lightVertices[i].pdfRev));
        for (int i = 0; i < nCamera; i++)
            CHECK(!std::isnan(cameraVertices[i].pdfFwd) && !std::isnan(cameraVertices[i].pdfRev));
        for (int t = 1; t <= nCamera; ++t) {
            for (int s = 0; s <= nLight; ++s) {
                int depth = t + s - 2;
                if ((s == 1 && t == 1) || depth < 0 || depth > maxDepth)
                    continue;
                Point2i raster;
                Float weight;
                Spectrum LPath = removeNaNs(
                        connectBDPT(scene, ctx, lightVertices, cameraVertices, s, t, &raster, &weight));

                if (t != 1)
                    L += LPath;
                else if (!LPath.isBlack()) {
                    std::lock_guard<std::mutex> lockGuard(filmMutex);
                    film.addSplat(raster, LPath);
                }

#ifdef BDPT_DEBUG
                {
                    std::lock_guard<std::mutex> lockGuard(mutex);
                    if (t != 1)
                        debugFilms[std::make_pair(s, t)].addSample(ctx.raster, LPath);
                    else
                        debugFilms[std::make_pair(s, t)].addSplat(raster, LPath);
                }
#endif
            }
        }
        film.addSample(ctx.raster, L);
    });
}

int BDPT::randomWalk(Ray ray, Scene &scene, RenderContext &ctx, Spectrum beta, Float pdf, int maxDepth, Vertex *path,
                     TransportMode mode) {
    if (maxDepth <= 0)
        return 0;
    auto infos = ctx.arena.alloc<IntersectionInfo>((size_t) maxDepth);
    int depth = 0;
    Float pdfFwd = pdf, pdfRev = 0;
    Float R = beta.max();
    while (true) {
        Vertex &vertex = path[depth];
        Vertex &prev = path[depth - 1];
        auto info = &infos[depth];
        bool foundIntersection = scene.intersect(ray, info);
        if (!foundIntersection) { break; }
        ScatteringEvent event = makeScatteringEvent(ray, info, ctx.sampler);
        vertex = Vertex::createSurfaceVertex(event, beta, pdfFwd, prev);
        if (++depth >= maxDepth)
            break;
        auto f = info->bsdf->sample(event);
        pdfFwd = event.pdf;
        if (f.isBlack() || pdfFwd <= 0)
            break;
        beta *= f * Vec3f::absDot(event.Ns, event.wiW) / pdfFwd;
        pdfRev = info->bsdf->pdf(event.wi, event.wo, BSDFType::all);
        if ((int) event.sampledType & int(BSDFType::specular)) {
            vertex.isDelta = true;
            pdfFwd = pdfRev = 0;
        }
        // TODO: correct shading normal
        ray = event.spawnRay(event.wiW);
        prev.pdfRev = vertex.convertDensity(pdfRev, prev);
        if (depth >= scene.option.minDepth && beta.max() < R) {
            if (ctx.sampler->nextFloat() * R < beta.max()) {
                beta /= beta.max() / R;
            } else {
                break;
            }
        }

    }
    return depth;
}


int BDPT::generateCameraSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path) {
    auto &vertex = path[0];
    vertex = Vertex::createCameraVertex(ctx.primary, ctx.camera, {1, 1, 1});
    Float pdf, _;
    ctx.camera->pdfWe(ctx.primary, &_, &pdf);
    return randomWalk(ctx.primary, scene, ctx, {1, 1, 1}, pdf, maxDepth - 1, path + 1, importance) + 1;
}

int BDPT::generateLightSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path) {
    auto &vertex = path[0];
    Float lightPdf = 0;
    auto light = scene.chooseOneLight(*ctx.sampler, &lightPdf);
    Float pdfDir, pdfPos;
    Ray ray({}, {});
    Vec3f normal;
    auto Le = light->sampleLe(ctx.sampler->nextFloat2D(), ctx.sampler->nextFloat2D(), &ray, &normal, &pdfPos, &pdfDir);

    // lightPdf is included in Le
    auto beta = Spectrum(Le * Vec3f::dot(ray.d, normal) / (pdfDir * pdfPos));
    if (pdfDir <= 0 || pdfPos <= 0 || Le.isBlack() || beta.isBlack()) { return 0; }
    // TODO: infinite light
    vertex = Vertex::createLightVertex(light, ray, normal, lightPdf * pdfPos, Le);
    vertex.L = Le;
    vertex.pdfPos = pdfPos;
    return randomWalk(ray, scene, ctx, beta, pdfDir, maxDepth - 1, path + 1, radiance) + 1;
}

/*
 Multiple importance sampling using balanced heuristics
 Loop over all possible paths of a specific length to compute the weight

 \begin{align*}
  p_s(\overline{x}) &= p^{\rightarrow}(x_0) ... p^{\rightarrow}(x_{s-1})p^{\leftarrow}(x_{t-1}) ... p^{\leftarrow}(x_{n-1})\\
  p_i(\overline{x}) &= p^{\rightarrow}(x_0) ... p^{\rightarrow}(x_{i-1})p^{\leftarrow}(x_{i}) ... p^{\leftarrow}(x_{n-1})\\
  w(s) &= \dfrac{p_s(\overline{x})}{\sum\limits_ip_i(\overline{x})}
  = \dfrac{1}{\sum\limits_i\dfrac{p_s(\overline{x})}{p_i(\overline{x})}}\\
  &= \Bigg(
  \sum\limits_{i=0}^{s-1}\dfrac{p_s(\overline{x})}{p_i(\overline{x})}
  + 1
  + \sum\limits_{i=s+1}^{n}\dfrac{p_s(\overline{x})}{p_i(\overline{x})}
  \Bigg)^{-1}
\end{align*}

 */
Float BDPT::MISWeight(Scene &scene,
                      RenderContext &ctx,
                      Vertex *lightVertices,
                      Vertex *cameraVertices,
                      int s, int t,
                      Vertex &sampled) {
    if (s + t == 2)
        return 1;

    // Get the necessary vertices
    Vertex *qs = s > 0 ? &lightVertices[s - 1] : nullptr,
            *pt = t > 0 ? &cameraVertices[t - 1] : nullptr,
            *qsMinus = s > 1 ? &lightVertices[s - 2] : nullptr,
            *ptMinus = t > 1 ? &cameraVertices[t - 2] : nullptr;

    // Temporarily update vertex
    ScopedAssignment<Vertex> a1;
    if (s == 1) {
        a1 = {qs, sampled};
    } else if (t == 1) {
        a1 = {pt, sampled};
    }

    ScopedAssignment<bool> a2, a3;
    if (pt)
        a2 = {&pt->isDelta, false};
    if (qs)
        a3 = {&qs->isDelta, false};

    ScopedAssignment<Float> a4;
    if (pt) {
        a4 = {&pt->pdfRev, s > 0 ? qs->pdf(scene, qsMinus, *pt) : pt->pdfLightOrigin(scene, *ptMinus)};
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

    // Remaps the delta pdf
    auto remap0 = [](Float x) { return x != 0 ? x : 1; };
    Float sumRi = 0;
    Float ri = 1;
    for (int i = t - 1; i > 0; i--) { //???
        ri *= remap0(cameraVertices[i].pdfRev) / remap0(cameraVertices[i].pdfFwd);
        if (!cameraVertices[i].isDelta && !cameraVertices[i - 1].isDelta)
            sumRi += ri;
    }
    ri = 1;
    for (int i = s - 1; i >= 0; i--) {
        ri *= remap0(lightVertices[i].pdfRev) / remap0(lightVertices[i].pdfFwd);
        bool delta = i > 0 ? lightVertices[i - 1].isDelta : lightVertices[0].light->isDeltaLight();
        if (!lightVertices[i].isDelta && !delta)
            sumRi += ri;
    }
    return 1.0f / (1.0f + sumRi);
}

/*
 The naive strategy uses weight = 1 / path length
 The estimator is unbiased since the sum of weights of a particular path length is 1
 Though it doesn't give good result
 */
Spectrum
BDPT::connectBDPT(Scene &scene, RenderContext &ctx, Vertex *lightVertices, Vertex *cameraVertices, int s, int t,
                  Point2i *raster, Float *misWeightPtr) {
    // TODO: infinite lights

    Vertex &E = cameraVertices[t - 1];
    Spectrum Li;
    Vertex sampled;
    if (misWeightPtr) {
        *misWeightPtr = 0;
    }
    if (s == 0) {
        Vertex &prev = cameraVertices[t - 2];
        Li = prev.beta * E.Le(prev);
    } else {
        Vertex &L = lightVertices[s - 1];

        if (t == 1) {
            if (!L.connectable())return {};
            // Try rasterizing the point
            if (!ctx.camera->rasterize(L.hitPoint(), raster))
                return {};
            auto wi = (L.hitPoint() - ctx.camera->viewpoint);
            auto dist = wi.lengthSquared();
            wi /= sqrt(dist);
            Ray primary(ctx.camera->viewpoint, wi);
            Spectrum beta(1, 1, 1); // TODO: We should use `sampleWe` instead.
            Float pdf, _;
            ctx.camera->pdfWe(primary, &_, &pdf);
            sampled = Vertex::createCameraVertex(primary, ctx.camera, Spectrum(beta / pdf));
            sampled.pdfFwd = pdf;
            Li = L.beta * L.f(sampled) / dist;
            if (Li.isBlack())return {};
            OccludeTester tester(primary, sqrt(dist));
            if (!tester.visible(scene))return {};
            Li *= Vec3f::absDot(primary.d, L.Ns());
        } else if (s == 1) {
//            Vec3f wi = (L.hitPoint() - E.hitPoint()).normalized();
//            Li = L.L * E.beta * E.f(L);
//            Li /= L.pdfPos;
//            if (Li.isBlack()) return {};
//            Li *= GWithoutAbs(scene, ctx, L, E);
//            if (Li.isBlack())return {};

//            VisibilityTester tester;
//            tester.targetGeomID = E.event.getIntersectionInfo()->geomID;
//            tester.targetPrimID = E.event.getIntersectionInfo()->primID;
//            tester.shadowRay = Ray(L.hitPoint(), -1 * wi);
//            if (!tester.visible(scene))return {};
//            sampled = Vertex::createLightVertex(L.light, tester.shadowRay, L.lightNormal, L.pdfPos, L.beta);
//            sampled.pdfFwd = sampled.pdfLightOrigin(scene, E);
            Vec3f wi;
            Float pdf;
            VisibilityTester tester;
            Li = L.light->sampleLi(ctx.sampler->nextFloat2D(), *E.event.getIntersectionInfo(), &wi, &pdf, &tester);
            if (Li.isBlack() || pdf <= 0)return {};
            sampled = Vertex::createLightVertex(L.light, tester.shadowRay, L.Ng(), pdf, Spectrum(Li / pdf));
            if (!tester.visible(scene))return {};
            Li *= E.f(sampled) / pdf * Vec3f::absDot(wi, E.Ns());
            sampled.pdfFwd = sampled.pdfLightOrigin(scene, E);
        } else {
            if (!L.connectable() || !E.connectable())return {};
            Vec3f wi = (L.hitPoint() - E.hitPoint()).normalized();
            Li = L.beta * E.beta * L.f(E) * E.f(L);
            if (Li.isBlack()) return {};
            Li *= G(scene, ctx, L, E);
            VisibilityTester tester;
            tester.targetGeomID = E.event.getIntersectionInfo()->geomID;
            tester.targetPrimID = E.event.getIntersectionInfo()->primID;
            tester.shadowRay = Ray(L.hitPoint(), -1 * wi);
            if (!tester.visible(scene))return {};
        }
    }
    if (Li.isBlack())return Li;
    if (s + t == 2)return Li;
    Float naiveWeight = 1.0f / (s + t);
    Float weight = MISWeight(scene, ctx, lightVertices, cameraVertices, s, t, sampled);
    CHECK(!std::isnan(weight) && weight >= 0);
    if (misWeightPtr) {
        *misWeightPtr = weight;
    }
    Li *= weight;
    return Li;
}

Spectrum BDPT::G(Scene &scene, RenderContext &ctx, Vertex &L, Vertex &E) {
    Vec3f wi = (L.hitPoint() - E.hitPoint());
    Float dist = wi.lengthSquared();
    Float g = 1 / dist;
    wi /= sqrt(dist);
    g *= Vec3f::absDot(wi, L.Ns());
    g *= Vec3f::absDot(wi, E.Ns());
    Spectrum beta(1, 1, 1);
    beta *= g;
    return beta;
}

Spectrum BDPT::GWithoutAbs(Scene &scene, RenderContext &ctx, Vertex &L, Vertex &E) {
    Vec3f wi = (L.hitPoint() - E.hitPoint());
    Float dist = wi.lengthSquared();
    Float g = 1 / dist;
    wi /= sqrt(dist);
    g *= -Vec3f::dot(wi, L.Ns());
    g *= Vec3f::dot(wi, E.Ns());
    Spectrum beta(1, 1, 1);
    beta *= g;
    return beta;
}


bool Vertex::connectable() const {
    switch (type) {
        case Vertex::lightVertex:
            return ((int) light->type & (int) Light::Type::deltaDirection) == 0;
        case Vertex::surfaceVertex:
            return event.getIntersectionInfo()->bsdf->matchFlags(
                    BSDFType::allButSpecular);

        case Vertex::cameraVertex:
            return true;
    }
    return false;
}

Float Vertex::convertDensity(Float pdf, const Vertex &next) const {
    // TODO: infinite light
    // solid angle = dA cos(theta) / dist^2
    // 1/ solid angle = 1 / dA  * dist^2 / cos(theta)
    // pdf = pdfA * dist^2 / cos(theta)
    // pdfA = pdf / dist^2 * cos(theta)
    // TODO: justify the math
    Vec3f w = next.hitPoint() - hitPoint();
    Float invDist = 1.0f / w.lengthSquared();
    pdf *= Vec3f::absDot(next.Ng(), w * sqrt(invDist));
    return pdf * invDist;
}

Float Vertex::pdfLightOrigin(Scene &scene, const Vertex &v) const {
    CHECK(!isInfiniteLight());
    CHECK(light);
    auto w = (v.hitPoint() - hitPoint()).normalized();
    auto pdfChoice = scene.pdfLightChoice(light);
    Float pdfPos, pdfDir;
    light->pdfLe({hitPoint(), w}, Ng(), &pdfPos, &pdfDir);
    return pdfPos * pdfChoice;
}


Float Vertex::pdfLight(Scene &scene, const Vertex &v) const {
    auto w = v.hitPoint() - hitPoint();
    Float invDist2 = 1 / w.lengthSquared();
    w *= sqrt(invDist2);
    Float pdf;
    if (isInfiniteLight()) {
        // TODO
        return 1 / (PI * scene.worldRadius() * scene.worldRadius());
    } else {
        CHECK(light);
        Float pdfPos, pdfDir;
        light->pdfLe(Ray(hitPoint(), w), Ng(), &pdfPos, &pdfDir);
        pdf = pdfDir * invDist2;
    }
    pdf *= Vec3f::absDot(v.Ng(), w);
    return pdf;
}

Float Vertex::pdf(Scene &scene, const Vertex *prev, const Vertex &next) const {
    if (type == Vertex::lightVertex) {
        return pdfLight(scene, next);
    }
    Vec3f wp, wn;
    wn = (next.hitPoint() - hitPoint()).normalized();
    if (prev) {
        wp = (prev->hitPoint() - hitPoint()).normalized();
    } else {
        CHECK(type == Vertex::cameraVertex);
    }
    Float pdf;
    if (type == Vertex::surfaceVertex) {
        pdf = event.getIntersectionInfo()->bsdf->pdf(event.worldToLocal(wp), event.worldToLocal(wn), BSDFType::all);

    } else {
        assert(type == Vertex::cameraVertex);
        Float _;
        camera->pdfWe({hitPoint(), wn}, &_, &pdf);
    }
    return convertDensity(pdf, next);
}

Vertex
Vertex::createSurfaceVertex(const ScatteringEvent &event, const Spectrum &beta, Float pdfFwd, const Vertex &prev) {
    Vertex vertex;
    vertex.event = event;
    vertex.beta = beta;
    vertex.type = surfaceVertex;
    vertex.light = event.getIntersectionInfo()->primitive->light;
    // Critical, Vertex::convertDensity requires Vertex::type field to be initialized
    vertex.pdfFwd = prev.convertDensity(pdfFwd, vertex);
    return vertex;
}

Vertex
Vertex::createCameraVertex(const Ray &primary, Camera *camera, const Spectrum &beta) {
    Vertex vertex;
    vertex.beta = beta;
    vertex.camera = camera;
    vertex.primary = primary;
    vertex.type = cameraVertex;
    vertex.isDelta = false;
    return vertex;

}

Vertex Vertex::createLightVertex(Light *light, const Ray &ray, const Vec3f &normal, Float pdf, const Spectrum &beta) {
    Vertex vertex;
    vertex.beta = beta;
    vertex.light = light;
    vertex.type = lightVertex;
    vertex.primary = ray;
    vertex.lightNormal = normal;
    vertex.pdfFwd = pdf;
    return vertex;
}

