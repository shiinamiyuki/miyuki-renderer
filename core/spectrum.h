//
// Created by Shiina Miyuki on 2019/1/16.
//

#ifndef MIYUKI_SPECTRUM_H
#define MIYUKI_SPECTRUM_H

#include "util.h"
#include "geometry.h"

namespace Miyuki {
    template<size_t N>
    class CoefficientSpectrum : public Vec<Float, N> {
    public:

    };
    template<>
    class CoefficientSpectrum<3>: public Vec3f{};
    class RGBSpectrum : public CoefficientSpectrum<3> {
    public:
        explicit RGBSpectrum(const Vec3f&c){
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
        }
        explicit RGBSpectrum(const Vec<Float,3> &c){
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
        }
        RGBSpectrum(Float x = 0, Float y = 0, Float z = 0) {
            v[0] = x;
            v[1] = y;
            v[2] = z;
        }
        RGBSpectrum &operator = (const Vec3f&c){
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
            return *this;
        }
        RGBSpectrum &operator = (const Vec<Float,3>&c){
            v[0] = c[0];
            v[1] = c[1];
            v[2] = c[2];
            return *this;
        }
        // performs gamma correction and maps the output to [0, 255)
        RGBSpectrum gammaCorrection() const;
    };

    using Spectrum = RGBSpectrum;
}
#endif //MIYUKI_SPECTRUM_H
