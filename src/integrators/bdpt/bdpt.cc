//
// Created by Shiina Miyuki on 2019/1/22.
//

#include "bdpt.h"
#include "../../core/scene.h"
#include "../../core/film.h"
#include "../../math/geometry.h"
#include "../../samplers/random.h"
#include "../../lights/light.h"

using namespace Miyuki;

void Miyuki::BDPT::render(Scene &scene) {
    fmt::print("Rendering\n");
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int32_t N = scene.option.samplesPerPixel;
    int32_t sleepTime = scene.option.sleepTime;
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
}

// IMPORTANT! What we are doing here is based on the assumption that during each pass the memory allocated
// is never freed
void BDPT::iteration(Scene &scene) {
    auto &film = scene.film;
    auto &seeds = scene.seeds;
    int maxDepth = (int) scene.option.maxDepth;
    scene.foreachPixel([&](RenderContext &ctx) {
        Spectrum L;
        auto cameraVertices = ctx.arena.alloc<Vertex>(maxDepth + 2u);
        auto lightVertices = ctx.arena.alloc<Vertex>(maxDepth + 1u);
        auto nCamera = generateCameraSubpath(scene, ctx, maxDepth + 2, cameraVertices);
        auto nLight = generateLightSubpath(scene, ctx, maxDepth + 1, lightVertices);
        for (int t = 1; t <= nCamera; ++t) {
            for (int s = 0; s <= nLight; ++s) {
                int depth = t + s - 2;
                if ((s == 1 && t == 1) || depth < 0 || depth > maxDepth)
                    continue;
              //  if (!(s == 1))continue;
                Point2i rasterNew;
                Point2f raster;
                Spectrum LPath = connectBDPT(scene, ctx, lightVertices, cameraVertices, s, t, &raster, nullptr);
                if (t != 1)
                    L += LPath;
                else {
                    film.addSplat(rasterNew, LPath);
                }
            }
        }
        film.addSample(ctx.raster, L);
    });

}

int BDPT::randomWalk(Ray ray, Scene &scene, RenderContext &ctx, Spectrum beta, Float pdf, int maxDepth, Vertex *path) {
    if (maxDepth <= 0)
        return 0;
    auto infos = ctx.arena.alloc<IntersectionInfo>((size_t) maxDepth);
    int depth = 0;
    Float pdfFwd = pdf, pdfRev = 0;
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
    }
    return depth;
}


int BDPT::generateCameraSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path) {
    auto &vertex = path[0];
    vertex = Vertex::createCameraVertex(ctx.primary, ctx.camera, {1, 1, 1});
    Float pdf, _;
    ctx.camera->pdfWe(ctx.primary, &_, &pdf);
    return randomWalk(ctx.primary, scene, ctx, {1, 1, 1}, pdf, maxDepth - 1, path + 1) + 1;
}

int BDPT::generateLightSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path) {
    auto &vertex = path[0];
    auto light = scene.chooseOneLight(*ctx.sampler);
    Float pdfDir, pdfPos;
    Ray ray({}, {});
    Vec3f normal;
    auto Le = light->sampleLe(ctx.sampler->nextFloat2D(), ctx.sampler->nextFloat2D(), &ray, &normal, &pdfPos, &pdfDir);
    auto beta = Spectrum(Le * Vec3f::dot(ray.d, normal) / (pdfDir * pdfPos));
    if (pdfDir == 0 || Le.isBlack())return 0;
    // TODO: infinite light
    vertex = Vertex::createLightVertex(light, ray, normal, Le);
    vertex.L = Le;
    vertex.pdfPos = pdfPos;
    return randomWalk(ray, scene, ctx, beta, pdfDir, maxDepth - 1, path + 1) + 1;
}

/*
 The naive strategy uses weight = 1 / path length
 The estimator is unbiased since the sum of weights of a particular path length is 1
 Though it doesn't give good result
 */
Spectrum
BDPT::connectBDPT(Scene &scene, RenderContext &ctx, Vertex *lightVertices, Vertex *cameraVertices, int s, int t,
                  Point2f *raster, Float *misWeight) {
    auto maxDepth = scene.option.maxDepth;
    // TODO: infinite lights
    Float weight = 1.0f / (s + t - 1);
    Vertex &E = cameraVertices[t - 1];
    Spectrum Li;
    if (s == 0) {
        Vertex &prev = cameraVertices[t - 2];
        Li = prev.beta * E.Le(prev);
    } else {
        Vertex &L = lightVertices[s - 1];
        if (t == 1) {
            return {};
        } else if (s == 1) {
            Vec3f wi = (L.hitPoint() - E.hitPoint()).normalized();
            Li = L.L * E.beta * E.f(L);
            Li /= L.pdfPos;
            if (Li.isBlack()) return {};
            Li *= GWithoutAbs(scene, ctx, L, E);
            if(Li.isBlack())return {};
            VisibilityTester tester;
            tester.targetGeomID = E.event.getIntersectionInfo()->geomID;
            tester.targetPrimID = E.event.getIntersectionInfo()->primID;
            tester.shadowRay = Ray(L.hitPoint(), -1 * wi);
            if (!tester.visible(scene))return {};
        } else {
            if(Li.isBlack())return {};
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
    Spectrum beta(1,1,1);
    beta *= g;
    return beta ;
}

Spectrum BDPT::GWithoutAbs(Scene &scene, RenderContext &ctx, Vertex &L, Vertex &E) {
    Vec3f wi = (L.hitPoint() - E.hitPoint());
    Float dist = wi.lengthSquared();
    Float g = 1 / dist;
    wi /= sqrt(dist);
    g *= -Vec3f::dot(wi, L.Ns());
    g *= Vec3f::dot(wi, E.Ns());
    Spectrum beta(1,1,1);
    beta *= g;
    return beta ;
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
    Vec3f w = next.hitPoint() - next.hitPoint();
    Float invDist = 1.0f / w.lengthSquared();
    pdf *= Vec3f::absDot(next.Ng(), w * sqrt(invDist));
    return pdf * invDist;
}

Float Vertex::pdf(const Scene &scene, const Vertex *prev, const Vertex &next) const {
    Vec3f wp, wn;
    wn = (next.hitPoint() - event.hitPoint()).normalized();
    if (prev) {
        wp = (prev->hitPoint() - event.hitPoint()).normalized();
    }
    Float pdf;
    if (type == Vertex::surfaceVertex) {
        pdf = event.getIntersectionInfo()->bsdf->pdf(wp, wn, event.sampledType);

    } else if (type == Vertex::lightVertex) {
        return 0;
    } else {
        CHECK(type == Vertex::cameraVertex);
        Float _;
        camera->pdfWe(event.spawnRay(wn), &_, &pdf);
    }
    return convertDensity(pdf, next);
}

Vertex
Vertex::createSurfaceVertex(const ScatteringEvent &event, const Spectrum &beta, Float pdfFwd, const Vertex &prev) {
    Vertex vertex;
    vertex.event = event;
    vertex.beta = beta;
    vertex.pdfFwd = pdfFwd;
    vertex.pdfFwd = prev.convertDensity(pdfFwd, vertex);
    vertex.type = surfaceVertex;
    return vertex;
}

Vertex
Vertex::createCameraVertex(const Ray &primary, Camera *camera, const Spectrum &beta) {
    Vertex vertex;
    vertex.beta = beta;
    vertex.camera = camera;
    vertex.primary = primary;
    vertex.type = cameraVertex;
    return vertex;

}

Vertex Vertex::createLightVertex(Light *light, const Ray &ray, const Vec3f &normal, const Spectrum &beta) {
    Vertex vertex;
    vertex.beta = beta;
    vertex.light = light;
    vertex.type = lightVertex;
    vertex.primary = ray;
    vertex.lightNormal = normal;
    return vertex;
}
