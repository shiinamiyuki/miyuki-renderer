//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_SPECTRUM_H
#define MIYUKI_SPECTRUM_H

#include "geometry.h"
#include <io/serialize.h>

namespace Miyuki {
    template<size_t N>
    class CoefficientSpectrum : public Vec<Float, N> {
    public:

    };

    template<>
    class CoefficientSpectrum<3> : public Vec3f {
    };

    class RGBSpectrum : public CoefficientSpectrum<3> {
    public:
        RGBSpectrum(const Vec3f &c) {
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
        }

        RGBSpectrum(const Vec<Float, 3> &c) {
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
        }

        RGBSpectrum(Float x = 0) {
            v[0] = x;
            v[1] = x;
            v[2] = x;
        }

        RGBSpectrum(Float x, Float y, Float z) {
            v[0] = x;
            v[1] = y;
            v[2] = z;
        }

        RGBSpectrum &operator=(const Vec3f &c) {
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
            return *this;
        }

        RGBSpectrum &operator=(const Vec<Float, 3> &c) {
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
            return *this;
        }

        // performs gamma correction and maps the output to [0, 255)
        RGBSpectrum gammaCorrection() const;

        bool hasNaNs() const;

        bool nonNeg() const {
            return r() >= 0 && g() >= 0 && b() >= 0;
        }

        bool isBlack() const;

        inline Float luminance() const {
            return Float(0.299) * r() + Float(0.587) * g() + Float(0.114) * b();
        }
    };

    RGBSpectrum removeNaNs(const RGBSpectrum &);

    using Spectrum = RGBSpectrum;
    namespace IO {
        template<>
        inline Json::JsonObject serialize<Spectrum>(const Spectrum &v) {
            return serialize<Vec3f>(v);
        }
        template<>
        inline Spectrum deserialize<Spectrum>(const Json::JsonObject &v) {
            return deserialize<Vec3f>(v);
        }
    }
    inline Spectrum clampRadiance(const Spectrum &s, Float maxR) {
        return {clamp<Float>(s.r(), 0, maxR), clamp<Float>(s.g(), 0, maxR), clamp<Float>(s.b(), 0, maxR)};
    }
}
#endif //MIYUKI_SPECTRUM_H
