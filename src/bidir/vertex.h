//
// Created by xiaoc on 2019/3/10.
//

#ifndef MIYUKI_VERTEX_H
#define MIYUKI_VERTEX_H

#include "miyuki.h"
#include "core/scatteringevent.h"
#include "lights/light.h"

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
            const Light *light;
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
        };

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
        };

        struct Path {
            SubPath cameraPath;
            SubPath lightPath;

            Float MISWeight() const;

            Spectrum L() const;
        };
    }
}
#endif //MIYUKI_VERTEX_H
