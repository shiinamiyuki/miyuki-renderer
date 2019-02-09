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

    struct Vertex {
        enum VertexType {
            lightVertex,
            cameraVertex,
            surfaceVertex,
        };
        Vec3f hitPoint, normal, radiance;
        Float pdfFwd, pdfRev;
        BSDFType sampledType;
        VertexType type;
        int32_t geomID;
        int32_t primID;

        Vertex(VertexType _type) : geomID(-1), primID(-1), type(_type) {}

        bool connectable(const Vertex &rhs) const;

        Float convertDensity(Float pdf, const Vertex &) const;

        Float pdf(const Scene &, const Vertex *, const Vertex &next)const;
    };

    class Path : public std::vector<Vertex> {

    };

    class BDPT : public Integrator {
    public:


    protected:
        void generateLightPath(Sampler &, Scene &, Path &, uint32_t maxS);

        void generateEyePath(RenderContext &ctx, Scene &, Path &, uint32_t maxT);

        Spectrum connectBDPT(Scene &, Path &L, Path &E, int32_t s, int32_t t);

        void iteration(Scene &scene);

    public:
        void render(Scene &) override;
    };
}

#endif //MIYUKI_BDPT_H
