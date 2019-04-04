//
// Created by Shiina Miyuki on 2019/3/5.
//

#include "scatteringevent.h"
#include "mesh.h"
#include "materials/material.h"
#include <core/ray.h>

namespace Miyuki {
    ScatteringEvent::ScatteringEvent(Sampler *sampler, Intersection *isct, BSDF *bsdf, TransportMode mode)
            : u(sampler->get2D()), intersection(isct), coordinateSystem(isct->Ns), bsdf(bsdf), mode(mode), pdf(0) {
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

    void ScatteringEvent::computeRayDifferentials(const RayDifferential &ray) {
        if (ray.hasDifferential) {
            auto d = -Vec3f::dot(Ng(), getIntersection()->p);
            Float tx = (-Vec3f::dot(Ng(), ray.rxOrigin) - d) / Vec3f::dot(Ng(), ray.rxDirection);
            Float ty = (-Vec3f::dot(Ng(), ray.ryOrigin) - d) / Vec3f::dot(Ng(), ray.ryDirection);
            auto px = ray.rxOrigin + tx * ray.rxDirection;
            auto py = ray.ryOrigin + ty * ray.ryDirection;
            const auto &p = getIntersection()->p;
            dpdx = px - p;
            dpdy = py - p;

            // select two dimensions
            int dim[2];
            if (std::abs(Ng().x()) > std::abs(Ng().y()) && std::abs(Ng().x()) > std::abs(Ng().z())) {
                dim[0] = 1;
                dim[1] = 2;
            } else if (std::abs(Ng().y()) > std::abs(Ng().z())) {
                dim[0] = 0;
                dim[1] = 2;
            } else {
                dim[0] = 0;
                dim[1] = 1;
            }

            const auto &dpdu = getIntersection()->dpdu;
            const auto &dpdv = getIntersection()->dpdv;


            Float A[2][2] = {{dpdu[dim[0]], dpdv[dim[0]]},
                             {dpdu[dim[1]], dpdv[dim[1]]}};
            Float Bx[2] = {px[dim[0]] - p[dim[0]],
                           px[dim[1]] - p[dim[1]]};
            Float By[2] = {py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]]};

            if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx))
                dudx = dvdx = 0;

            if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy))
                dudy = dvdy = 0;

        } else {
            dudx = dvdx = 0;
            dudy = dvdy = 0;
            dpdx = dpdy = Vec3f();
        }
    }
}
