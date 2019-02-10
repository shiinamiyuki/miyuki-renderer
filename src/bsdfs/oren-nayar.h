//
// Created by Shiina Miyuki on 2019/2/9.
//

#ifndef MIYUKI_OREN_NAYAR_H
#define MIYUKI_OREN_NAYAR_H

#include "bsdf.h"
namespace Miyuki{
    class OrenNayarBSDF : public BSDF {
        Float A, B;
    public:
        OrenNayarBSDF(Float sigma, const ColorMap &albedo, const ColorMap &bump = ColorMap())
                : BSDF(BSDFType((int) BSDFType::diffuse | (int) BSDFType::reflection), albedo, bump) {
            Float sigma2 = sigma * sigma;
            A = 1 - sigma2 / ((sigma2 + 0.33f) * 2.0f);
            B = 0.45f * sigma2 / (sigma2 + 0.09f);
        }

        Spectrum f(const ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_OREN_NAYAR_H
