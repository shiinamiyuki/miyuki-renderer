//
// Created by xiaoc on 2019/1/16.
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

    class RGBSpectrum : public CoefficientSpectrum<3> {
    public:
        RGBSpectrum(Float x, Float y, Float z) {
            v[0] = x;
            v[1] = y;
            v[2] = z;
        }

        RGBSpectrum gammaCorrection() const;
    };

    using Spectrum = RGBSpectrum;
}
#endif //MIYUKI_SPECTRUM_H
