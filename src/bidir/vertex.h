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
            Float pdfFwd, pdfRev;
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
                        break;
                    case mediumVertex:
                        return true;;
                }
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
        };

        inline Vertex CreateSurfaceVertex(ScatteringEvent *event, Float pdf, Spectrum beta, const Vertex &prev) {
            Vertex v;
            v.type = Vertex::surfaceVertex;
            v.primitive = event->getIntersection()->primitive;
            v.event = event;
            v.beta = beta;
            v.pdfFwd = prev.convertDensity(pdf, v);
            v.light = event->getIntersection()->primitive->light;
            v.Ns = event->Ns();
            v.Ng = event->Ng();
            v.ref = event->getIntersection()->ref;
            return std::move(v);
        }

        inline Vertex CreateLightVertex(Light *light, const Ray &ray, const Vec3f &normal, Float pdf, Spectrum beta) {
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
        CreateCameraVertex(Camera *camera, const Point2i &raster, const Ray &ray, Float pdf, Spectrum beta) {
            Vertex v;
            v.type = Vertex::cameraVertex;
            v.camera = camera;
            v.beta = beta;
            v.pdfFwd = pdf;
            v.raster = raster;
            v.Ns = v.Ng = ray.d; // TODO
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

        struct Path {
            SubPath cameraPath;
            SubPath lightPath;

            Float MISWeight() const;

            Spectrum L() const;
        };

        SubPath
        RandomWalk(Vertex *, Ray ray,
                   Spectrum beta, Float pdf, Scene &scene,
                   RenderContext &ctx, int minDepth, int maxDepth, TransportMode);
    }
}
#endif //MIYUKI_VERTEX_H
