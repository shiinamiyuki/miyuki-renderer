//
// Created by Shiina Miyuki on 2019/3/6.
//

#include "lambertian.h"
#include "core/scatteringevent.h"
#include <math/sampling.h>

namespace Miyuki {

    Spectrum LambertianReflection::f(const ScatteringEvent &event) const {
        return R * INVPI;
    }

    Point2f LambertianReflection::invert(const Vec3f &wo, const Vec3f &wi) const {
        return InvertCosineWeightedHemisphereSampling(wi);
    }
    
}