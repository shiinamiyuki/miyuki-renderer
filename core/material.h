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

namespace Miyuki {
    enum class BxDFType {
        none = 0,
        diffuse = 1,
        specular = 2, // this field is for all specular transmissions, including specular/glossy reflection/refraction
        refraction = 4, //this field indicates we are sampling refraction not reflection
        emission = 8,
        glossy = 16,
        all = diffuse | specular | emission | glossy,
    };

    inline bool hasBxDFType(BxDFType query, BxDFType ty) {
        return (bool) ((int) query & (int) ty);
    }

    inline bool isDeltaDistribution(BxDFType type) {
        return hasBxDFType(type, BxDFType::specular) && !hasBxDFType(type, BxDFType::glossy);
    }

    struct Interaction;

    class Material {
    public:
        // TODO: make these private
        Spectrum ka, kd, ks;
        Float glossiness, ior, tr;
        std::unique_ptr<TextureMapping2D> kaMap, kdMap, ksMap;
    private:
        Spectrum kaAt(const Point2f & uv)const;
        Spectrum kdAt(const Point2f & uv)const;
        Spectrum ksAt(const Point2f & uv)const;
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

    };

    inline Vec3f reflect(const Vec3f &norm, const Vec3f &i) {
        return i - 2 * Vec3f::dot(norm, i) * norm;
    }
}
#endif //MIYUKI_MATERIAL_H
