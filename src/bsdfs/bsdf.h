//
// Created by Shiina Miyuki on 2019/3/5.
//

#ifndef MIYUKI_BSDF_H
#define MIYUKI_BSDF_H

#include "core/geometry.h"
#include "core/spectrum.h"

namespace Miyuki {
    inline bool SameHemisphere(const Vec3f &w1, const Vec3f &w2) {
        return w1.y() * w2.y() >= 0;
    }

    inline Float CosTheta(const Vec3f &w) {
        return w.y();
    }

    inline Float AbsCosTheta(const Vec3f &w) {
        return fabs(CosTheta(w));
    }

    inline Float Cos2Theta(const Vec3f &w) {
        return w.y() * w.y();
    }

    inline Float Sin2Theta(const Vec3f &w) {
        return std::max(Float(0), 1 - Cos2Theta(w));
    }

    inline Float SinTheta(const Vec3f &w) {
        return std::sqrt(Sin2Theta(w));
    }

    inline Float TanTheta(const Vec3f &w) {
        return SinTheta(w) / CosTheta(w);
    }

    inline Float Tan2Theta(const Vec3f &w) {
        return Sin2Theta(w) / Cos2Theta(w);
    }

    inline Float CosPhi(const Vec3f &w) {
        auto s = SinTheta(w);
        return s == 0 ? 1.0f : clamp(w.x() / s, -1.0f, 1.0f);
    }

    inline Float SinPhi(const Vec3f &w) {
        auto s = SinTheta(w);
        return s == 0 ? 0.0f : clamp(w.z() / s, -1.0f, 1.0f);
    }

    inline Float Cos2Phi(const Vec3f &w) {
        auto c = CosPhi(w);
        return c * c;
    }

    inline Float Sin2Phi(const Vec3f &w) {
        auto s = SinPhi(w);
        return s * s;
    }

    inline Float CosDPhi(const Vec3f &wa, const Vec3f &wb) {
        return clamp<Float>((wa.x() * wb.x() + wa.z() * wb.z()) /
                            std::sqrt((wa.x() * wa.x() + wa.z() * wa.z()) *
                                      (wb.x() * wb.x() + wb.z() * wb.z())), -1.0f, 1.0f);
    }

    inline Vec3f Reflect(const Vec3f &wo, const Vec3f &n) {
        return -1 * wo + 2 * Vec3f::dot(wo, n) * n;
    }

    inline Float FrDielectric(Float cosThetaI, Float etaI, Float etaT) {
        cosThetaI = clamp(cosThetaI, -1, 1);
        bool entering = cosThetaI > 0.f;
        if (!entering) {
            std::swap(etaI, etaT);
            cosThetaI = std::abs(cosThetaI);
        }
        Float sinThetaI = std::sqrt(std::max((Float) 0,
                                             1 - cosThetaI * cosThetaI));
        Float sinThetaT = etaI / etaT * sinThetaI;
        if (sinThetaT >= 1)
            return 1;
        Float cosThetaT = std::sqrt(std::max((Float) 0,
                                             1 - sinThetaT * sinThetaT));

        Float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                      ((etaT * cosThetaI) + (etaI * cosThetaT));
        Float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                      ((etaI * cosThetaI) + (etaT * cosThetaT));
        return (Rparl * Rparl + Rperp * Rperp) / 2;
    }

    struct BSDFLobe {
        static const uint32_t none = 0;
        static const uint32_t reflection = 1;
        static const uint32_t transmission = 2;
        static const uint32_t diffuse = 4;
        static const uint32_t glossy = 8;
        static const uint32_t specular = 16;
        static const uint32_t all = reflection | transmission | glossy | specular | diffuse;
        static const uint32_t allButSpecular = all & ~specular;

        operator int() const {
            return value;
        }

        BSDFLobe() : value(none) {}

        BSDFLobe(int value) : value(value) {}

        bool matchFlag(const BSDFLobe &rhs) const {
            return (value & rhs.value) != 0;
        }

    private:
        uint32_t value;
    };

    class ScatteringEvent;

    class BSDF;

    class BxDF {
    public:
        const BSDFLobe lobe;

        BxDF(const BSDFLobe &lobe) : lobe(lobe) {}

        virtual Spectrum f(const ScatteringEvent &event) const = 0;

        virtual Float pdf(const ScatteringEvent &event) const;

        virtual Spectrum sample(ScatteringEvent &event) const;

        virtual Point2f invert(const Vec3f &wo, const Vec3f &wi) const = 0;

        virtual ~BxDF() {}

        bool matchFlag(const BSDFLobe &rhs) const {
            return lobe.matchFlag(rhs);
        }
    };

    class BSDF {
    public:
        static const int maxBxDFs = 8;
    protected:
        int nBxDFs = 0;
        BxDF *bxdfs[maxBxDFs];
        BSDFLobe lobe = BSDFLobe::none;
    public:
        void add(BxDF *bxdf) {
            bxdfs[nBxDFs++] = bxdf;
            lobe = lobe | bxdf->lobe;
        }

        BSDFLobe getLobe() const {
            return lobe;
        }

        BSDF() {}

        Spectrum f(const ScatteringEvent &event, BSDFLobe lobe = BSDFLobe::all) const;

        Float pdf(const ScatteringEvent &event, BSDFLobe lobe = BSDFLobe::all) const;

        Spectrum sample(ScatteringEvent &event, BSDFLobe lobe = BSDFLobe::all) const;

        void invert(const ScatteringEvent &event, Point2f *u, Float *pdf) const;

        int numComponents(BSDFLobe lobe) const {
            int n = 0;
            for (int i = 0; i < nBxDFs; i++) {
                if (bxdfs[i]->lobe.matchFlag(lobe))n++;
            }
            return n;
        }
    };

    class Fresnel {
    public:
        virtual Spectrum eval(Float cosI) const = 0;
    };

    class PerfectSpecularFresnel : public Fresnel {
    public:
        Spectrum eval(Float cosI) const override {
            return {1, 1, 1};
        }
    };

    class FresnelDielectric : public Fresnel {
        Float etaI, etaT;
    public:
        FresnelDielectric(Float etaI, Float etaT) : etaI(etaI), etaT(etaT) {}

        Spectrum eval(Float cosI) const override {
            auto x = FrDielectric(cosI, etaI, etaT);
            return {x, x, x};
        }
    };

    inline bool Refract(const Vec3f &wi, const Vec3f &n, Float eta,
                        Vec3f *wt) {
        Float cosThetaI = Vec3f::dot(n, wi);
        Float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
        Float sin2ThetaT = eta * eta * sin2ThetaI;
        if (sin2ThetaT >= 1) return false;

        Float cosThetaT = std::sqrt(1 - sin2ThetaT);

        *wt = -eta * wi + (eta * cosThetaI - cosThetaT) * Vec3f(n);
        return true;
    }
}

#endif //MIYUKI_BSDF_H
