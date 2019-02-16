//
// Created by Shiina Miyuki on 2019/1/22.
//

#ifndef MIYUKI_BDPT_H
#define MIYUKI_BDPT_H

#include "../integrator.h"
#include "../../math/geometry.h"
#include "../../samplers/sampler.h"
#include "../../core/spectrum.h"
#include "../../core/intersection.h"
#include "../../core/scene.h"

namespace Miyuki {
    enum class BSDFType;

    class Light;

    struct Vertex {
        enum VertexType {
            lightVertex,
            cameraVertex,
            surfaceVertex,
        };
        ScatteringEvent event;
        Spectrum beta;
        Float pdfFwd, pdfRev;
        VertexType type;
        bool isDelta;
        Float pdfPos;
        Spectrum L;
        Vec3f lightNormal;
        Light *light;
        Camera *camera;
        Ray primary;


        Vertex() : light(nullptr), primary({}, {}) {}

        bool connectable() const;

        Float convertDensity(Float pdf, const Vertex &) const;

        Float pdf(const Scene &, const Vertex *, const Vertex &next) const;

        static Vertex
        createLightVertex(Light *light, const Ray &ray, const Vec3f &normal,Float pdf,  const Spectrum &beta);

        static Vertex
        createCameraVertex(const Ray &primary, Camera *, const Spectrum &beta);

        static Vertex
        createSurfaceVertex(const ScatteringEvent &event, const Spectrum &beta, Float pdfFwd, const Vertex &prev);

        Vec3f hitPoint() const {
            if (type == lightVertex || type == cameraVertex)
                return primary.o;
            else
                return event.hitPoint();
        }

        Vec3f Ng() const {
            if (type == lightVertex)
                return lightNormal;
            else if (type == cameraVertex) {
                return {};
            } else
                return event.Ng();
        }

        Vec3f Ns() const {
            if (type == lightVertex)
                return lightNormal;
            else if (type == cameraVertex) {
                return {};
            } else
                return event.Ns;
        }

        Spectrum f(const Vertex &next) const {
            auto wi = (next.hitPoint() - hitPoint()).normalized();
            auto e = event;
            e.wiW = wi;
            e.wi = e.worldToLocal(e.wiW);
            switch (type) {
                case surfaceVertex:
                    return event.getIntersectionInfo()->bsdf->eval(e);
            }
            return {};
        }

        Spectrum Le(const Vertex &prev) const {
            auto wo = (prev.hitPoint() - hitPoint()).normalized();
            return event.Le(wo);
        }
    };

    class BDPT : public Integrator {
    public:

    protected:
        int generateCameraSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path);

        int generateLightSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path);

        int randomWalk(Ray ray, Scene &scene, RenderContext &ctx, Spectrum beta, Float pdf, int maxDepth, Vertex *path);

        void iteration(Scene &scene);

        Spectrum connectBDPT(Scene &scene,
                             RenderContext &ctx,
                             Vertex *lightVertices,
                             Vertex *cameraVertices,
                             int s,
                             int t,
                             Point2f *raster,
                             Float *misWeight = nullptr);

        Spectrum G(Scene &scene, RenderContext &ctx, Vertex &L, Vertex &E);

        Spectrum GWithoutAbs(Scene &scene, RenderContext &ctx, Vertex &L, Vertex &E);

    public:
        void render(Scene &) override;
    };

    template<typename T>
    struct ScopedAssignment {
        T *target;
        T backup;

        ScopedAssignment(T *target = nullptr) : target(target) { backup = *target; }

        ScopedAssignment(const ScopedAssignment &) = delete;

        ScopedAssignment &operator=(ScopedAssignment &&rhs) noexcept {
            if (target) {
                *target = backup;
            }
            target = rhs.target;
            backup = rhs.backup;
            rhs.backup = nullptr;
        }

        ~ScopedAssignment() {
            if (target)*target = backup;
        }
    };
}

#endif //MIYUKI_BDPT_H
