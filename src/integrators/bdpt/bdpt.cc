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
static DECLARE_STATS(uint32_t, pathCount);
static DECLARE_STATS(uint32_t, zeroPathCount);

void Miyuki::BDPT::render(Scene &scene) {
    fmt::print("Rendering\n");
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int32_t N = scene.option.samplesPerPixel;
    int32_t sleepTime = scene.option.sleepTime;
    auto maxDepth = scene.option.maxDepth;
    pathCount = 0;
    zeroPathCount = 0;
#ifdef BDPT_DEBUG
    for (int t = 1; t <= maxDepth + 2; ++t) {
        for (int s = 0; s <= maxDepth + 12; ++s) {
            debugFilms[std::make_pair(s, t)] = Film(scene.film.width(), scene.film.height());
        }
    }
#endif
    double elapsed = 0;
    for (int32_t i = 0; i < N && scene.processContinuable(); i++) {
        auto t = runtime([&]() {
            iteration(scene);
            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
        });
        elapsed += t;
        fmt::print("iteration {} in {} secs, elapsed {}s, remaining {}s, zero-path: {}%\n",
                   1 + i, t, elapsed, (double) (elapsed * N) / (i + 1) - elapsed,
                   (double) zeroPathCount / (double) pathCount * 100);
        scene.update();
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
        ctx.sampler->start();
        auto lightVertices = ctx.arena.alloc<Vertex>(maxDepth + 1u);
        ctx.sampler->start();
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
                Spectrum LPath =
                        connectBDPT(scene, ctx, lightVertices, cameraVertices, s, t, &raster, &weight);
                UPDATE_STATS(pathCount, 1);
                if (LPath.isBlack()) {
                    UPDATE_STATS(zeroPathCount, 1);
                }
                if (t != 1)
                    L += LPath;
                else if (!LPath.isBlack()) {
//                    std::lock_guard<std::mutex> lockGuard(filmMutex);
                    film.addSplat(raster, LPath);
                }

#ifdef BDPT_DEBUG
                {
                    std::lock_guard<std::mutex> lockGuard(mutex);
                    Spectrum out = LPath;
                    if (t != 1)
                        debugFilms[std::make_pair(s, t)].addSample(ctx.raster, out);
                    else
                        debugFilms[std::make_pair(s, t)].addSplat(raster, out);
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
        if (!foundIntersection) {
            // capture escaped rays from camera
            if (mode == TransportMode::radiance) {
                // add infinite light
                vertex = Vertex::createLightVertex(scene.infiniteLight.get(), ray, ray.d, pdfFwd, beta);
                depth++;
            }
            break;
        }
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
        // correct shading normal
        beta *= correctShadingNormal(event, event.woW, event.wiW, mode);
        ray = event.spawnRay(event.wiW);
        prev.pdfRev = vertex.convertDensity(pdfRev, prev);
        Float p = continuationProbability(scene, R, beta, depth);
        if (ctx.sampler->nextFloat() < p) {
            beta /= p;
        } else {
            break;
        }

    }
    return depth;
}

Float BDPT::continuationProbability(const Scene &scene, Float R, const Spectrum &beta, int depth) {
    if (depth + 1 >= scene.option.minDepth) {
        return std::min(1.0f, beta.max() / R);
    }
    return 1;
}


int BDPT::generateCameraSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path) {
    if (maxDepth == 0) {
        return 0;
    }
    auto &vertex = path[0];
    vertex = Vertex::createCameraVertex(ctx.primary, ctx.camera, {1, 1, 1});
    Float pdf, _;
    ctx.camera->pdfWe(ctx.primary, &_, &pdf);
    return randomWalk(ctx.primary, scene, ctx, {1, 1, 1}, pdf, maxDepth - 1, path + 1, TransportMode::radiance) + 1;
}

int BDPT::generateLightSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path) {
    if (maxDepth == 0) {
        return 0;
    }
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
    // At present, no rays will be emitted from infinite area lights.
    vertex = Vertex::createLightVertex(light, ray, normal, lightPdf * pdfPos, Le);
    vertex.pdfPos = pdfPos;
    return randomWalk(ray, scene, ctx, beta, pdfDir, maxDepth - 1, path + 1, TransportMode::importance) + 1;
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
    auto remap0 = [](Float x) { return x != 0 ? x * x : 1; };
    Float sumRi = 0;
    Float ri = 1;
    for (int i = t - 1; i > 0; i--) {
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
    // Ignore nonsensical connections related to infinite lights
    if (t > 1 && s != 0 && cameraVertices[t - 1].isInfiniteLight())
        return {};
    Vertex &E = cameraVertices[t - 1];
    Spectrum Li;
    Vertex sampled;
    if (misWeightPtr) {
        *misWeightPtr = 0;
    }
    if (s == 0) {
        Vertex &prev = cameraVertices[t - 2];
        Li = E.Le(prev) * E.beta;
        //At present, the only way to sample infinite light is to sample the brdf
        if (E.isInfiniteLight())
            return Li;
    } else {
        Vertex &L = lightVertices[s - 1];
        // nightmare
        if (t == 1) {
            if (!L.connectable())return {};
            // Try rasterizing the point
            if (!ctx.camera->rasterize(L.hitPoint(), raster))
                return {};
            auto wi = (L.hitPoint() - ctx.camera->viewpoint);
            auto dist = wi.lengthSquared();
            wi /= sqrt(dist);
            Ray primary(ctx.camera->viewpoint, wi);
            Spectrum beta = ctx.camera->We(primary); // TODO: We should use `sampleWe` instead.
            Float pdf, _;
            pdf = dist / Vec3f::absDot(ctx.camera->normal, wi);
            sampled = Vertex::createCameraVertex(primary, ctx.camera, Spectrum(beta / pdf));
            sampled.pdfFwd = pdf;
            Li = L.beta * L.f(sampled, TransportMode::importance) * sampled.beta;
            if (Li.isBlack())return {};
            OccludeTester tester(primary, sqrt(dist));
            if (!tester.visible(scene))return {};
            Li *= Vec3f::absDot(primary.d, L.Ns()) * Vec3f::dot(primary.d, ctx.camera->normal);
        } else if (s == 1) {
            if (!E.connectable())return {};
            Vec3f wi;
            Float pdf;
            VisibilityTester tester;
            // dynamically sample the light source
            auto light = L.light;
            Li = light->sampleLi(ctx.sampler->nextFloat2D(), *E.event.getIntersectionInfo(), &wi, &pdf, &tester);
            if (Li.isBlack() || pdf <= 0)return {};
            sampled = Vertex::createLightVertex(light, tester.shadowRay, L.Ng(), pdf, Spectrum(Li / pdf));
            Li *= E.beta * E.f(sampled, TransportMode::radiance) / pdf * Vec3f::absDot(wi, E.Ns());
            if (Li.isBlack())return {};
            if (!tester.visible(scene))return {};
            sampled.pdfFwd = sampled.pdfLightOrigin(scene, E);
        } else {
            if (L.connectable() && E.connectable()) {
                Li = L.beta * E.beta * L.f(E, TransportMode::importance) * E.f(L, TransportMode::radiance);
                if (Li.isBlack())return Li;
                Li *= G(scene, ctx, L, E);
                VisibilityTester tester;
                tester.shadowRay = Ray{L.hitPoint(), (E.hitPoint() - L.hitPoint()).normalized()};
                tester.targetPrimID = E.event.getIntersectionInfo()->primID;
                tester.targetGeomID = E.event.getIntersectionInfo()->geomID;
                if (!tester.visible(scene))return {};
            }
        }
    }
    if (Li.isBlack())return {};
    if (s + t == 2)return Li;
    Float naiveWeight = 1.0f / (s + t);
    Float weight = MISWeight(scene, ctx, lightVertices, cameraVertices, s, t, sampled);
    CHECK(!std::isnan(weight));
    CHECK(weight >= 0);
    if (misWeightPtr) {
        *misWeightPtr = weight;
    }
    Li *= weight;
    Li = clampRadiance(removeNaNs(Li), scene.option.maxRayIntensity);
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
    // solid angle density for infinite light
    if (isInfiniteLight())
        return pdf;
    // solid angle = dA cos(theta) / dist^2
    // 1/ solid angle = 1 / dA  * dist^2 / cos(theta)
    // pdf = pdfA * dist^2 / cos(theta)
    // pdfA = pdf / dist^2 * cos(theta)
    // TODO: justify the math
    Vec3f w = next.hitPoint() - hitPoint();
    Float invDist = 1.0f / w.lengthSquared();
    pdf *= Vec3f::absDot(next.Ng(), w.normalized());
    return pdf * invDist;
}

Float Vertex::pdfLightOrigin(Scene &scene, const Vertex &v) const {
    // TODO: infinite lights
    // Current implementation doesn't allow rays to be emitted from infinite area lights
    if (isInfiniteLight())
        return 0;
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
        // TODO: infinite lights
        return 0;
        //return 1 / (PI * scene.worldRadius() * scene.worldRadius());
    } else {
        CHECK(light);
        Float pdfPos, pdfDir;
        light->pdfLe(Ray(hitPoint(), w), Ng(), &pdfPos, &pdfDir);
        pdf = pdfDir * invDist2;
        CHECK(pdfDir >= 0);
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

Spectrum Vertex::f(const Vertex &next, TransportMode mode) const {

    auto wi = (next.hitPoint() - hitPoint()).normalized();
    auto e = event;
    e.wiW = wi;
    e.wi = e.worldToLocal(e.wiW);
    switch (type) {
        case surfaceVertex:
            return Spectrum(event.getIntersectionInfo()->bsdf->eval(e)
                            * BDPT::correctShadingNormal(e, e.woW, e.wiW, mode));
    }
    return {};

}

