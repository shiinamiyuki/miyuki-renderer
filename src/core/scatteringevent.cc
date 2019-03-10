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

    static inline Float mod(Float a, Float b) {
        int k = a / b;
        Float x = a - k * b;
        if (x < 0)
            x += b;
        if (x >= b)
            x -= b;
        return x;
    }

    const Point2f ScatteringEvent::textureUV() const {
        auto _uv = PointOnTriangle(intersection->primitive->textureCoord[0],
                                   intersection->primitive->textureCoord[1],
                                   intersection->primitive->textureCoord[2],
                                   intersection->uv[0],
                                   intersection->uv[1]);
        _uv.x() = mod(_uv.x(), 1);
        _uv.y() = mod(_uv.y(), 1);
        return _uv;
    }
}
