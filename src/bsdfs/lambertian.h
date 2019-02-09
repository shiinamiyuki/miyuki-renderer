//
// Created by Shiina Miyuki on 2019/2/9.
//

#ifndef MIYUKI_LAMBERTIAN_H
#define MIYUKI_LAMBERTIAN_H

#include "bsdf.h"
namespace Miyuki{
    class LambertianBSDF : public BSDF {
    public:
        LambertianBSDF(const ColorMap &albedo, const ColorMap &bump = ColorMap())
                : BSDF(BSDFType((int) BSDFType::diffuse | (int) BSDFType::reflection), albedo, bump) {}

        Spectrum brdf(const ScatteringEvent &event) const override;
    };
}
#endif //MIYUKI_LAMBERTIAN_H
