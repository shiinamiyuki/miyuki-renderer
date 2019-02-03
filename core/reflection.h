//
// Created by Shiina Miyuki on 2019/2/3.
//

#ifndef MIYUKI_REFLECTION_H
#define MIYUKI_REFLECTION_H

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

    class BxDF {
        const BxDFType type;
    public:
        bool matchFlags(BxDFType) const;

        virtual Spectrum sampleF(const Vec3f &wo, Vec3f *wi,
                                 const Point2f &sample, Float *pdf,
                                 BxDFType *sampledType = nullptr) const;

        virtual Spectrum f(const Vec3f &wo, const Vec3f &wi) const = 0;

        virtual Float Pdf(const Vec3f &wi, const Vec3f &wo) const;

//        virtual Spectrum rho(const Vec3f &wo, int nSamples,
//                             const Point2f *samples) const;
//
//        virtual Spectrum rho(int nSamples, const Vec3f *samples1,
//                             const Point2f *samples2) const;
    };

    class BSDF {
    private:
        static constexpr int maxBxDFs = 8;
        BxDF *bxdfs[maxBxDFs];
    public:
        const Float eta;

        Spectrum f(const Vec3f &woW, const Vec3f &wiW, BxDFType flags = BSDF_ALL) const;

        Spectrum sampleF(const Vec3f &wo, Vec3f *wi, const Point2f &u,
                         Float *pdf, BxDFType type = BSDF_ALL,
                         BxDFType *sampledType = nullptr) const;

        Float Pdf(const Vec3f &wo, const Vec3f &wi,
                  BxDFType flags = BSDF_ALL) const;
    };
}
#endif //MIYUKI_REFLECTION_H
