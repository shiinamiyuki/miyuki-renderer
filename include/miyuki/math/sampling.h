//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_SAMPLING_H
#define MIYUKI_SAMPLING_H

#include "miyuki.h"
#include "vec.hpp"

namespace Miyuki {
    Point2f ConcentricSampleDisk(const Point2f &u);

    inline Vec3f CosineWeightedHemisphereSampling(const Point2f &u) {
        Float theta = u[0] * 2 * PI;
        Float r = sqrtf(u[1]);
        return Vec3f(sin(theta) * r,  cos(theta) * r, sqrt(1 - r * r));
    }

    inline Vec3f UniformTriangleSampling(const Point2f &u0, const Vec3f &v0, const Vec3f &v1, const Vec3f &v2) {
        Float u = u0[0], v = u0[1];
        if (u + v >= 1) {
            u = 1 - u;
            v = 1 - v;
        }
        return PointOnTriangle(v0, v1, v2, u, v);
    }

	inline Vec3f UniformTriangleSampling(const Point2f& u0, 
		const Vec3f& v0,
		const Vec3f& v1,
		const Vec3f& v2,
		Point2f* uv) {
		Float u = u0[0], v = u0[1];
		if (u + v >= 1) {
			u = 1 - u;
			v = 1 - v;
		}
		*uv = { u,v };
		return PointOnTriangle(v0, v1, v2, u, v);
	}

    inline Vec3f UniformSampleSphere(const Point2f &u) {
        Float z = 1 - 2 * u[0];
        Float r = std::sqrt(std::max((Float) 0, (Float) 1 - z * z));
        Float phi = 2 * PI * u[1];
        return Vec3f(r * std::cos(phi), r * std::sin(phi), z);
    }

    inline Float UniformSpherePdf() {
        return 1.0f / (PI * 4);
    }

	inline Float CosineHemispherePdf(Float cosTheta) {
		return cosTheta * INVPI;
	}
}

#endif //MIYUKI_SAMPLING_H
