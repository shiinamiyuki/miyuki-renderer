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
        return s == 0 ? 1.0f : clamp(w.x() / s, -1.0f, -1.0f);
    }

    inline Float SinPhi(const Vec3f &w) {
        auto s = SinTheta(w);
        return s == 0 ? 1.0f : clamp(w.z() / s, -1.0f, -1.0f);
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
    public:
        void add(BxDF *bxdf) {
            bxdfs[nBxDFs++] = bxdf;
        }

        BSDF() {}

        Spectrum f(const ScatteringEvent &event, BSDFLobe lobe = BSDFLobe::all) const;

        Float pdf(const ScatteringEvent &event, BSDFLobe lobe = BSDFLobe::all) const;

        Spectrum sample(ScatteringEvent &event, BSDFLobe lobe = BSDFLobe::all) const;

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
}

#endif //MIYUKI_BSDF_H
