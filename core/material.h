//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H

#include "util.h"
#include "geometry.h"
#include "spectrum.h"
#include "sampler.h"
#include "interaction.h"
#include "texture.h"
#include "reflection.h"

namespace Miyuki {


    struct Interaction;

    class Material {
    public:
        // TODO: make these private
        Spectrum ka, kd, ks;
        Float glossiness, ior, tr;
        std::shared_ptr<TextureMapping2D> kaMap, kdMap, ksMap;
    private:
        Spectrum kaAt(const Interaction &) const;

        Spectrum kdAt(const Interaction &) const;

        Spectrum ksAt(const Interaction &) const;

    public:


        Material() {}

        // samples BRDF, returns color, pdf w.r.t direction and wi
        Spectrum sampleF(Sampler &sampler,
                         const Interaction &,
                         const Vec3f &wo,
                         Vec3f *wi,
                         Float *_pdf,
                         BxDFType sampleType = BxDFType::all,
                         BxDFType *_sampled = nullptr) const;

        // BRDF, given world space wo and wi
        Float f(BxDFType type, const Interaction &, const Vec3f &wo, const Vec3f &wi) const;

        const Point2f textCoord(const Interaction &) const;

    };

    inline Vec3f reflect(const Vec3f &normal, const Vec3f &i) {
        return i - 2 * Vec3f::dot(normal, i) * normal;
    }
}
#endif //MIYUKI_MATERIAL_H
