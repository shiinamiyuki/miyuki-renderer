//
// Created by Shiina Miyuki on 2019/2/3.
//

#ifndef MIYUKI_REFLECTION_H
#define MIYUKI_REFLECTION_H

#include "../utils/util.h"
#include "../math/geometry.h"
#include "../core/spectrum.h"
#include "../samplers/sampler.h"
#include "../core/scatteringevent.h"
#include "../core/texture.h"

namespace Miyuki {
    struct Interaction;
    struct ColorMap;
    enum class BSDFType {
        reflection = 1 << 0,
        transmission = 1 << 1,
        diffuse = 1 << 2,
        glossy = 1 << 3,
        specular = 1 << 4,
        all = diffuse | glossy | specular |
              reflection | transmission,
    };

    inline bool hasBxDFType(BSDFType query, BSDFType ty) {
        return (bool) ((int) query & (int) ty);
    }

    inline bool isDeltaDistribution(BSDFType type) {
        return hasBxDFType(type, BSDFType::specular) && !hasBxDFType(type, BSDFType::glossy);
    }


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

    struct MaterialInfo {
        ColorMap ka, kd, ks, bump;
        Float Ni, Ns, Tr;
        Float glossiness;
        std::string bsdfType;
        std::map<std::string, std::string> parameters;

        MaterialInfo(const ColorMap &ka, const ColorMap &kd, const ColorMap &ks, const ColorMap &bump = ColorMap()) :
                ka(ka), kd(kd), ks(ks), bump(bump), Ni(1), Ns(0), Tr(0) {}
    };

    class ScatteringEvent;

    class BSDF {
    protected:
        BSDFType type;
        ColorMap albedo, bump;

        virtual Spectrum f(const ScatteringEvent &) const = 0;

    public:
        ColorMap ka;

        BSDF(BSDFType type, const ColorMap &albedo, const ColorMap &bump = ColorMap())
                : type(type), albedo(albedo) {}

        virtual Spectrum sample(ScatteringEvent &) const;

        Spectrum eval(const ScatteringEvent &) const;

        virtual Float pdf(const Vec3f &wo, const Vec3f &wi) const;

        Spectrum evalAlbedo(const ScatteringEvent &) const;

        const ColorMap &Ka() const {
            return ka;
        }
    };



}
#endif //MIYUKI_REFLECTION_H
