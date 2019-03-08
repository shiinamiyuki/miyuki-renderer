//
// Created by Shiina Miyuki on 2019/3/7.
//

#include <core/mesh.h>
#include "area.h"
#include "math/sampling.h"

namespace Miyuki {

    Float AreaLight::power() const {
        return ka.albedo.max() * primitive->area;
    }

    Spectrum AreaLight::L() const {
        return ka.albedo;
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
        Float solidAngle = primitive->area * Vec3f::absDot(w, primitive->Ng) * invDist2;
        *pdf = 1.0f / solidAngle;
        *wi = w;
        return L();

    }

    Float AreaLight::pdfLi(const Intersection &intersection, const Vec3f &wi) const {
        Ray ray(intersection.ref, wi);
        Intersection isct;
        if (!primitive->intersect(ray, &isct)) {
            return 0.0f;
        }
        Float invDist2 = 1.0f / (isct.hitDistance() * isct.hitDistance());
        Float solidAngle = primitive->area * Vec3f::absDot(wi, primitive->Ng) * invDist2;
        return 1.0f / solidAngle;
    }
}