//
// Created by Shiina Miyuki on 2019/3/31.
//

#include "toon.h"
#include <core/scatteringevent.h>

namespace Miyuki {

    Spectrum ToonDiffuse::f(const ScatteringEvent &event) const {
        auto theta = std::acos(AbsCosTheta(event.wi));
        if (theta > size * (PI / 2)) {
            return 0.5f * R * INVPI / AbsCosTheta(event.wi);
        }
        return {};
    }
}
