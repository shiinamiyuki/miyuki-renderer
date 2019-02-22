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

    class Film;

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


        Vertex() : light(nullptr), camera(nullptr), primary({}, {}), pdfPos(NAN), pdfFwd(0), pdfRev(0),
                   isDelta(false) {}

        bool connectable() const;

        Float convertDensity(Float pdf, const Vertex &) const;

        Float pdf(Scene &, const Vertex *, const Vertex &next) const;

        static Vertex
        createLightVertex(Light *light, const Ray &ray, const Vec3f &normal, Float pdf, const Spectrum &beta);

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

        Spectrum f(const Vertex &next, TransportMode mode) const;

        Spectrum Le(const Vertex &prev) const {
            if (isInfiniteLight()) {
                return light->L();
            }
            auto wo = (prev.hitPoint() - hitPoint()).normalized();
            return event.Le(wo);
        }

        Float pdfLight(Scene &scene, const Vertex &v) const;

        Float pdfLightOrigin(Scene &scene, const Vertex &v) const;

        bool isInfiniteLight() const {
            return type == lightVertex && ((int) light->type & (int) Light::Type::infinite) != 0;
        }

        bool isOnSurface() const;
    };

    class BDPT : public Integrator {
    public:
        std::map<std::pair<int, int>, Film> debugFilms;
    protected:

        int generateCameraSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path);

        int generateLightSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path);

        int randomWalk(Ray ray, Scene &scene, RenderContext &ctx, Spectrum beta, Float pdf, int maxDepth, Vertex *path,
                       TransportMode mode);

        virtual void iteration(Scene &scene);

        Spectrum connectBDPT(Scene &scene,
                             RenderContext &ctx,
                             Vertex *lightVertices,
                             Vertex *cameraVertices,
                             int s,
                             int t,
                             Point2i *raster,
                             Float *misWeight = nullptr);

        Float MISWeight(Scene &scene,
                        RenderContext &ctx,
                        Vertex *lightVertices,
                        Vertex *cameraVertices,
                        int s,
                        int t,
                        Vertex &sampled);

        Spectrum G(Scene &scene, RenderContext &ctx, Vertex &L, Vertex &E);

        virtual Float continuationProbability(const Scene &scene, Float R, const Spectrum &beta, int depth);

    public:
        static inline Float
        correctShadingNormal(const ScatteringEvent &event, const Vec3f &wo, const Vec3f &wi, TransportMode mode) {
            if (mode == TransportMode::importance) {
                auto num = Vec3f::absDot(wo, event.Ns) * Vec3f::absDot(wi, event.Ng());
                auto denom = Vec3f::absDot(wo, event.Ng()) * Vec3f::absDot(wi, event.Ns);
                if(denom == 0)return 0;
                return num / denom;
            } else {
                return 1.0f;
            }
        }

        void render(Scene &) override;
    };

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

#endif //MIYUKI_BDPT_H
