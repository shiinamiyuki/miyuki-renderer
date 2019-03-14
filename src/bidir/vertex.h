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

namespace Miyuki {
    namespace Bidir {
        enum class TransportMode {
            radiance,
            importance
        };

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
            Point2i raster;
            const Camera *camera = nullptr;
            const Light *light = nullptr;
            const Primitive *primitive = nullptr;
            Float pdfFwd = -1, pdfRev = -1;
            bool delta = false;

            bool isInfiniteLight() const {
                return false;
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
                Assert(false);   return false;
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
                    return light->L();
                }
                return {};
            }

            Spectrum Le(const Vertex &prev) const {
                if (isInfiniteLight()) {
                    return light->L();
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
            v.light = event->getIntersection()->primitive->light;
            v.Ns = event->Ns();
            v.Ng = event->Ng();
            v.ref = event->getIntersection()->ref;
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
        CreateCameraVertex(const Camera *camera, const Point2i &raster, const Ray &ray, Float pdf, Spectrum beta) {
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
}
#endif //MIYUKI_VERTEX_H
