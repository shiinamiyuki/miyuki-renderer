//
// Created by Shiina Miyuki on 2019/3/10.
//

#ifndef MIYUKI_VERTEX_H
#define MIYUKI_VERTEX_H

#include <core/memory.h>
#include "miyuki.h"
#include "core/scatteringevent.h"
#include "lights/light.h"
#include <core/mesh.h>
#include <core/rendercontext.h>
#include <core/scene.h>

namespace Miyuki {
    namespace Bidir {

        inline Float
        CorrectShadingNormal(const ScatteringEvent &event, const Vec3f &wo, const Vec3f &wi, TransportMode mode) {
            // TODO: we might need to implement this differently from pbrt
            if (mode == TransportMode::importance) {
                auto num = Vec3f::absDot(wo, event.Ns()) * Vec3f::absDot(wi, event.Ng());
                auto denom = Vec3f::absDot(wo, event.Ng()) * Vec3f::absDot(wi, event.Ns());
                if (denom == 0)return 0;
                return num / denom;
            } else {
                return 1.0f;
            }
        }

        struct Vertex {
            enum Type {
                invalidVertex,
                lightVertex,
                cameraVertex,
                surfaceVertex,
                mediumVertex,
            } type = invalidVertex;
            Vec3f Ns;
            Vec3f Ng;
            Vec3f ref;
            Spectrum beta;
            ScatteringEvent *event = nullptr;
            Point2f raster;
            const Camera *camera = nullptr;
            const Light *light = nullptr;
            const Primitive *primitive = nullptr;
            Float pdfFwd = -1, pdfRev = -1;
            bool delta = false;

            bool isInfiniteLight() const {
                return light && ((light->type & (Light::infinite)) != 0);
            }

            bool onSurface() const {
                return primitive != nullptr;
            }

            bool connectable() const {
                switch (type) {
                    case invalidVertex:
                        throw std::runtime_error("invalid vertex!");
                    case lightVertex:
                        return true;
                    case cameraVertex:
                        break;
                    case surfaceVertex:
                        return !delta;
                    case mediumVertex:
                        return true;;
                }
                Assert(false);
                return false;
            }

            // area * cos / dist^2 = solid angle
            // area = solid angle * dist^2 / cos
            // 1/ area = 1/solid angle / dist^2 * cos
            Float convertDensity(Float pdf, const Vertex &other) const {
                if (isInfiniteLight())
                    return pdf;
                Vec3f w = (other.ref - ref);
                auto invDist2 = 1.0f / w.lengthSquared();
                w *= sqrtf(invDist2);
                return pdf * invDist2 * Vec3f::absDot(w, other.Ng);
            }

            Spectrum Le(const Vec3f &wo) const {
                if (type == surfaceVertex) {
                    return event->Le(wo);
                } else if (type == lightVertex) {
                    return light->L(Ray{ref, wo});
                }
                return {};
            }

            Spectrum Le(const Vertex &prev) const {
                if (isInfiniteLight()) {
                    return light->L(Ray{prev.ref, (ref - prev.ref).normalized()});
                }
                auto wo = (prev.ref - ref).normalized();
                return Le(wo);
            }

            Spectrum f(const Vertex &next, TransportMode mode) const {
                auto wi = (next.ref - ref).normalized();
                auto e = *event;
                e.wiW = wi;
                e.wi = e.worldToLocal(e.wiW);
                switch (type) {
                    case surfaceVertex:
                        return event->bsdf->f(e)
                               * CorrectShadingNormal(e, e.woW, e.wiW, mode);
                }
                return {};
            }

            Float pdfLight(Scene &scene, const Vertex &v) const {
                auto w = v.ref - ref;
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
                    light->pdfLe(Ray(ref, w), Ng, &pdfPos, &pdfDir);
                    pdf = pdfDir * invDist2;
                    CHECK(pdfDir >= 0);
                }

                pdf *= Vec3f::absDot(v.Ng, w);
                return pdf;
            }

            Float pdf(Scene &scene, const Vertex *prev, const Vertex &next) const {
                if (type == Vertex::lightVertex) {
                    return pdfLight(scene, next);
                }
                Vec3f wp, wn;
                wn = (next.ref - ref).normalized();
                if (prev) {
                    wp = (prev->ref - ref).normalized();
                } else {
                    CHECK(type == Vertex::cameraVertex);
                }
                Float pdf;
                if (type == Vertex::surfaceVertex) {
                    auto e = *event;
                    e.wo = event->worldToLocal(wp);
                    e.wi = event->worldToLocal(wn);
                    pdf = event->bsdf->pdf(e);

                } else {
                    assert(type == Vertex::cameraVertex);
                    Float _;
                    camera->pdfWe(Ray{ref, wn}, &_, &pdf);
                }
                return convertDensity(pdf, next);
            }

            Float pdfLightOrigin(Scene &scene, const Vertex &v) const;

        };

        inline Vertex CreateSurfaceVertex(ScatteringEvent *event, Float pdf, Spectrum beta, const Vertex &prev) {
            Vertex v;
            v.type = Vertex::surfaceVertex;
            v.primitive = event->getIntersection()->primitive;
            v.event = event;
            v.beta = beta;
            v.light = event->getIntersection()->primitive->light();
            v.Ns = event->Ns();
            v.Ng = event->Ng();
            v.ref = event->getIntersection()->p;
            v.pdfFwd = prev.convertDensity(pdf, v);
            return std::move(v);
        }

        inline Vertex
        CreateLightVertex(const Light *light, const Ray &ray, const Vec3f &normal, Float pdf, Spectrum beta) {
            Vertex v;
            v.type = Vertex::lightVertex;
            v.primitive = light->getPrimitive();
            v.beta = beta;
            v.pdfFwd = pdf;
            v.light = light;
            v.Ns = v.Ng = normal;
            v.ref = ray.o;
            return v;
        }

        inline Vertex
        CreateCameraVertex(const Camera *camera, const Point2f &raster, const Ray &ray, Float pdf, Spectrum beta) {
            Vertex v;
            v.type = Vertex::cameraVertex;
            v.camera = camera;
            v.beta = beta;
            v.pdfFwd = pdf;
            v.raster = raster;
            v.Ns = v.Ng = camera->cameraToWorld(Vec3f{0, 0, 1}); // TODO
            v.ref = ray.o;
            return v;
        }

        static Float G(const Vertex &v1, const Vertex &v2) {
            Vec3f w = (v1.ref - v2.ref);
            auto invDist2 = 1.0f / w.lengthSquared();
            w *= sqrtf(invDist2);
            Float G = invDist2 * Vec3f::absDot(w, v1.Ng) * Vec3f::absDot(w, v2.Ng);
            return G;
        }

        struct SubPath {
            Vertex *vertices = nullptr;
            int N = 0;

            SubPath(Vertex *v, int n) : vertices(v), N(n) {}

            SubPath sub(int i) {
                Assert(i <= N);
                return {vertices, i};
            }

            Vertex *begin() const {
                return vertices;
            }

            Vertex *end() const {
                return vertices + N;
            }

            Vertex &operator[](int i) {
                return vertices[i];
            }
        };

        SubPath
        RandomWalk(Vertex *, Ray ray,
                   Spectrum beta, Float pdf, Scene &scene,
                   RenderContext &ctx, int minDepth, int maxDepth, TransportMode);

        SubPath
        RandomWalk(Vertex *, Intersection *intersections, ScatteringEvent *events, Ray ray,
                   Spectrum beta, Float pdf, Scene &scene,
                   RenderContext &ctx, int minDepth, int maxDepth,
                   TransportMode, const std::function<Float(Spectrum)> &);

    }
    template<typename T>
    class ScopedAssignment {
        T *target;
        T backup;
    public:
        ScopedAssignment(T *target = nullptr, T value = T()) : target(target) {
            if (target) {
                backup = *target;
                *target = value;
            }
        }

        ScopedAssignment(const ScopedAssignment &) = delete;

        ScopedAssignment &operator=(const ScopedAssignment &) = delete;

        ScopedAssignment &operator=(ScopedAssignment &&rhs) noexcept {
            if (target) {
                *target = backup;
            }
            target = rhs.target;
            backup = rhs.backup;
            rhs.target = nullptr;
            return *this;
        }

        ~ScopedAssignment() {
            if (target)*target = backup;
        }
    };

    inline Float
    MISWeight(Scene &scene, RenderContext &ctx, Bidir::SubPath &lightSubPath, Bidir::SubPath &cameraSubPath,
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


        auto remap0 = [](Float x) -> Float { return x != 0 ? x * x : 1; };
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

    namespace Bidir {
        inline Spectrum
        ConnectBDPT(Scene &scene, RenderContext &ctx,
                    Bidir::SubPath &lightSubPath, Bidir::SubPath &cameraSubPath,
                    int s, int t, Point2f *raster, bool useMIS, Float *weight) {
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

        inline Bidir::SubPath GenerateLightSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth) {
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

        inline Bidir::SubPath GenerateCameraSubPath(Scene &scene, RenderContext &ctx, int minDepth, int maxDepth) {
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
    }
}
#endif //MIYUKI_VERTEX_H
