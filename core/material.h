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

namespace Miyuki {
    enum class BxDFType {
        none = 0,
        diffuse = 1,
        specular = 2,
        emission = 4,
        all = diffuse | specular | emission,
    };

    inline bool isDeltaDistribution(BxDFType type) {
        return (int) type & (int) BxDFType::specular;
    }
    struct Interaction;
    class Material {
    public:
        // TODO: make these private
        Spectrum ka, kd, ks;
        Float glossiness;
    private:

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

    };
    inline Vec3f reflect(const Vec3f&norm, const Vec3f& i){
        return i - 2 * Vec3f::dot(norm, i) * norm;
    }
}
#endif //MIYUKI_MATERIAL_H
