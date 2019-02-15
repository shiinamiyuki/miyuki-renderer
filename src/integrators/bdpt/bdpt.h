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
        Light *light;
        Camera * camera;
        Vertex() {}

        bool connectable(const Vertex &rhs) const;

        Float convertDensity(Float pdf, const Vertex &) const;

        Float pdf(const Scene &, const Vertex *, const Vertex &next) const;

        static Vertex
        createCameraVertex();

        static Vertex
        createSurfaceVertex(const ScatteringEvent &event, const Spectrum &beta, Float pdfFwd, const Vertex &prev);
    };

    class BDPT : public Integrator {
    public:

    protected:
        int generateCameraSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path);

        int generateLightSubpath(Scene &scene, RenderContext &ctx, int maxDepth, Vertex *path);

        int randomWalk(Ray ray, Scene &scene, RenderContext &ctx, Spectrum beta, Float pdf, int maxDepth, Vertex *path);

        void iteration(Scene &scene);

    public:
        void render(Scene &) override;
    };
}

#endif //MIYUKI_BDPT_H
