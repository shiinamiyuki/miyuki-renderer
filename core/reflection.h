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

        virtual Float Pdf(const Vec3f &wo, const Vec3f &wi) const;

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
        const Spectrum R;
    public:
        LambertianReflection(const Spectrum &R) : R(R),
                                                  BxDF(BxDFType(
                                                          (int) BxDFType::reflection | (int) BxDFType::diffuse)) {}

        Spectrum f(const Vec3f &wo, const Vec3f &wi) const override;
    };

    class OrenNayar : public BxDF {
        const Spectrum R;
        Float A, B;
    public:
        OrenNayar(const Spectrum &R, Float sigma) : R(R),
                                                    BxDF(BxDFType(
                                                            (int) BxDFType::reflection | (int) BxDFType::diffuse)) {
            Float sigma2 = sigma * sigma;
            A = 1 - sigma2 / ((sigma2 + 0.33f) * 2.0f);
            B = 0.45f * sigma2 / (sigma2 + 0.09f);
        }

        Spectrum f(const Vec3f &wo, const Vec3f &wi) const override;
    };

    class BSDF {
    private:
        static constexpr int maxBxDFs = 8;
        int nBxDFs;
        BxDF *bxdfs[maxBxDFs];
        Vec3f localX, localZ;
        Vec3f Ns, Ng;// shading normal, geometry normal
        void computeLocalCoordinates();

    public:
        const Float eta;

        void add(BxDF *);

        Vec3f worldToLocal(const Vec3f &v) const; // transform according to shading normal

        Vec3f localToWorld(const Vec3f &v) const;

        Spectrum f(const Vec3f &woW, const Vec3f &wiW, BxDFType flags = BxDFType::all) const;

        Spectrum sampleF(const Vec3f &woW, Vec3f *wiW, const Point2f &u,
                         Float *pdf, BxDFType type = BxDFType::all,
                         BxDFType *sampledType = nullptr) const;

        Float Pdf(const Vec3f &woW, const Vec3f &wiW,
                  BxDFType flags = BxDFType::all) const;

        int32_t numComponents(BxDFType flags) const;

        BSDF(const Interaction &);
    };

    inline Float cosTheta(const Vec3f &w) {
        return w.y();
    }

    inline Float absCosTheta(const Vec3f &w) {
        return fabs(w.y());
    }

    inline Float cos2Theta(const Vec3f &w) {
        return w.y() * w.y();
    }

    inline Float sin2Theta(const Vec3f &w) {
        return std::max(Float(0), 1 - cos2Theta(w));
    }

    inline Float sinTheta(const Vec3f &w) {
        return std::sqrt(sin2Theta(w));
    }

    inline Float tanTheta(const Vec3f &w) {
        return sinTheta(w) / cosTheta(w);
    }

    inline Float tan2Theta(const Vec3f &w) {
        return sin2Theta(w) / cos2Theta(w);
    }

    inline Float cosPhi(const Vec3f &w) {
        auto s = sinTheta(w);
        return s == 0 ? 1.0f : clamp(w.x() / s, -1.0f, -1.0f);
    }

    inline Float sinPhi(const Vec3f &w) {
        auto s = sinTheta(w);
        return s == 0 ? 1.0f : clamp(w.z() / s, -1.0f, -1.0f);
    }

    inline Float cos2Phi(const Vec3f &w) {
        auto c = cosPhi(w);
        return c * c;
    }

    inline Float sin2Phi(const Vec3f &w) {
        auto s = sinPhi(w);
        return s * s;
    }

    inline Float cosDPhi(const Vec3f &wa, const Vec3f &wb) {
        return clamp<Float>((wa.x() * wb.x() + wa.z() * wb.z()) /
                            std::sqrt((wa.x() * wa.x() + wa.z() * wa.z()) *
                                      (wb.x() * wb.x() + wb.z() * wb.z())), -1.0f, 1.0f);
    }
}
#endif //MIYUKI_REFLECTION_H
