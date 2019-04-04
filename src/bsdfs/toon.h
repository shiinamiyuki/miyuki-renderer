//
// Created by Shiina Miyuki on 2019/3/31.
//

#ifndef MIYUKI_TOON_H
#define MIYUKI_TOON_H

#include <bsdfs/bsdf.h>

namespace Miyuki {
    class ToonDiffuse : public BxDF {
        Spectrum R;
        Float size;
        Float smooth;
    public:
        ToonDiffuse(const Spectrum &R, Float size = 0.5f, Float smooth = 0.0f)
                : R(R), BxDF(BSDFLobe::diffuse | BSDFLobe::reflection),
                  size(size), smooth(smooth) {}

        Spectrum f(const ScatteringEvent &event) const override;
    };

    class ToonGlossy : public BxDF {

    };
}


#endif //MIYUKI_TOON_H
