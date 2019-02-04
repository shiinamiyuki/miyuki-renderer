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
    struct Interaction;
    struct ColorMap;
    enum class BxDFType {
        reflection = 1 << 0,
        transmission = 1 << 1,
        diffuse = 1 << 2,
        glossy = 1 << 3,
        specular = 1 << 4,
        all = diffuse | glossy | specular |
              reflection | transmission,
    };

    inline bool hasBxDFType(BxDFType query, BxDFType ty) {
        return (bool) ((int) query & (int) ty);
    }

    inline bool isDeltaDistribution(BxDFType type) {
        return hasBxDFType(type, BxDFType::specular) && !hasBxDFType(type, BxDFType::glossy);
    }

    class BxDF {

    public:
        const BxDFType type;

        BxDF(BxDFType t) : type(t) {}

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
        virtual ~BxDF() {}
    };

    class ScaledBxDF : public BxDF {
        Spectrum scale;
        BxDF *bxdf;
    public:
        ScaledBxDF(BxDF *bxdf, const Spectrum &scale) : BxDF(bxdf->type), bxdf(bxdf), scale(scale) {}

        Spectrum f(const Vec3f &wo, const Vec3f &wi) const override;

        Spectrum
        sampleF(const Vec3f &wo, Vec3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const override;

        Float Pdf(const Vec3f &wi, const Vec3f &wo) const override;
    };

    class LambertianReflection : public BxDF {
        const Spectrum &R;
    public:
        LambertianReflection(const Spectrum &R) : R(R),
                                                  BxDF(BxDFType(
                                                          (int) BxDFType::reflection | (int) BxDFType::diffuse)) {}

        Spectrum f(const Vec3f &wo, const Vec3f &wi) const override;
    };

    class BSDF {
    private:
        static constexpr int maxBxDFs = 8;
        int nBxDFs;
        BxDF *bxdfs[maxBxDFs];
    public:
        const Float eta;

        void add(BxDF *);

        Spectrum f(const Interaction &, BxDFType flags = BxDFType::all) const;

        Spectrum sampleF(const Vec3f &wo, Vec3f *wi, const Point2f &u,
                         Float *pdf, BxDFType type = BxDFType::all,
                         BxDFType *sampledType = nullptr) const;

        Float Pdf(const Vec3f &woW, const Vec3f &wiW,
                  const Vec3f &wo, const Vec3f &wi,
                  BxDFType flags = BxDFType::all) const;
    };
}
#endif //MIYUKI_REFLECTION_H
