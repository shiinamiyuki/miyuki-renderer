//
// Created by xiaoc on 2019/1/16.
//

#include "spectrum.h"

using namespace Miyuki;
static auto gamma = [](Float x) -> int {
    return lround(pow(clamp<Float>(x, 0.0, 1.0), 1.0 / 2.2) * 255.0);
};

RGBSpectrum RGBSpectrum::gammaCorrection() const {
    return RGBSpectrum(gamma(r()), gamma(g()), gamma(b()));
}
