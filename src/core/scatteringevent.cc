//
// Created by Shiina Miyuki on 2019/3/5.
//

#include "scatteringevent.h"
#include "mesh.h"
#include "materials/material.h"

namespace Miyuki {
    ScatteringEvent::ScatteringEvent(Sampler *sampler, Intersection *isct, BSDF *bsdf)
            : u(sampler->get2D()), intersection(isct), coordinateSystem(isct->Ns), bsdf(bsdf) {
        woW = isct->wo;
        wo = worldToLocal(woW);
    }

    Spectrum ScatteringEvent::Le(const Vec3f &wi) const {
        return intersection->Le(wi);
    }
}
