//
// Created by Shiina Miyuki on 2019/3/7.
//

#include <core/mesh.h>
#include "area.h"
#include "math/sampling.h"

namespace Miyuki {

    Float AreaLight::power() const {
        return ka.evalAlbedo().max() * primitive->area;
    }

    Spectrum AreaLight::L(const Ray &ray) const {
        return ka.evalAlbedo();
    }

    Spectrum AreaLight::sampleLi(const Point2f &u, const Intersection &intersection, Vec3f *wi, Float *pdf,
                                 VisibilityTester *tester) const {
        auto p = UniformTriangleSampling(u, primitive->v(0), primitive->v(1), primitive->v(2));
        Vec3f w = (p - intersection.ref);
        auto invDist2 = 1 / w.lengthSquared();
        w *= sqrtf(invDist2);
        tester->shadowRay = Ray{p, -1 * w};
        tester->geomId = intersection.geomId;
        tester->primId = intersection.primId;
        // convert area to solid angle
        Float solidAngle = primitive->area * -Vec3f::dot(w, primitive->Ng()) * invDist2;
        *pdf = std::max<Float>(0.0f, 1.0f / solidAngle);
        *wi = w;
        return ka.evalAlbedo();

    }

    Float AreaLight::pdfLi(const Intersection &intersection, const Vec3f &wi) const {
        Ray ray(intersection.ref, wi);
        Intersection isct;
        if (!primitive->intersect(ray, &isct)) {
            return 0.0f;
        }
        Float invDist2 = 1.0f / (isct.hitDistance() * isct.hitDistance());
        Float solidAngle = primitive->area * Vec3f::absDot(wi, primitive->Ng()) * invDist2;
        return 1.0f / solidAngle;
    }

    Spectrum AreaLight::sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                                 Float *pdfDir) const {
        auto p = UniformTriangleSampling(u1, primitive->v(0), primitive->v(1), primitive->v(2));
        *pdfPos = 1.0f / primitive->area;
        CoordinateSystem coordinateSystem(primitive->Ng());
        *normal = primitive->Ng();
        Vec3f wi = coordinateSystem.localToWorld(CosineWeightedHemisphereSampling(u2)).normalized();
        *pdfDir = Vec3f::absDot(wi, *normal) * INVPI;
        *ray = Ray(p, wi);
        return ka.albedo * ka.multiplier;
    }

    void AreaLight::pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const {
        *pdfPos = 1.0f / primitive->area;
        *pdfDir = std::max<Float>(0, Vec3f::dot(ray.d, normal) * INVPI);
    }
}