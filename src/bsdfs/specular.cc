//
// Created by Shiina Miyuki on 2019/3/8.
//

#include "specular.h"
#include "core/scatteringevent.h"

namespace Miyuki {

    Spectrum SpecularReflection::f(const ScatteringEvent &event) const {
        return {};
    }

    Float SpecularReflection::pdf(const ScatteringEvent &event) const {
        return 0;
    }

    Spectrum SpecularReflection::sample(ScatteringEvent &event) const {
        event.setWi(Vec3f{-event.wo.x(), event.wo.y(), -event.wo.z()});
        event.pdf = 1;
        event.bsdfLobe = lobe;
        return R / Vec3f::absDot(event.wiW, event.Ns());
    }
}