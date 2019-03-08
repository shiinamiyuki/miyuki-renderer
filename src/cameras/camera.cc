//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "camera.h"
#include "samplers/sampler.h"
#include "math/sampling.h"

namespace Miyuki {
    void Camera::moveTo(const Vec3f &v) {
        viewpot = v;
    }

    void Camera::move(const Vec3f &v) {
        viewpot += v;
    }

    void Camera::rotate(const Vec3f &v) {
        direction = v;
    }

    void Camera::rotateTo(const Vec3f &v) {
        direction += v;
    }

    void Camera::computeTransformMatrix() {
        rotationMatrix = Matrix4x4::rotation(Vec3f(0, 0, 1), direction.z());
        rotationMatrix = rotationMatrix.mult(Matrix4x4::rotation(Vec3f(0, 1, 0), direction.x()));
        rotationMatrix = rotationMatrix.mult(Matrix4x4::rotation(Vec3f(1, 0, 0), -direction.y()));
        Matrix4x4::inverse(rotationMatrix, invMatrix);
    }

    Float PerspectiveCamera::generateRay(Sampler &sampler, const Point2i &raster, Ray *ray, Float *weight) {
        Float x = -(2 * (Float) raster.x() / dimension.x() - 1) * static_cast<Float>(dimension.x()) /
                  dimension.y();
        Float y = 2 * (1 - (Float) raster.y() / dimension.y()) - 1;
        Float dx = 2.0f / dimension.y() / 2, dy = 2.0f / dimension.y() / 2;
        Vec3f ro(0, 0, 0);
        auto z = (Float) (2.0 / tan(fov / 2));
        Float rx = 2 * sampler.get1D() - 1;
        Float ry = 2 * sampler.get1D() - 1;
        Vec3f jitter = Vec3f(dx * rx, dy * ry, 0);
        // tent filter
        *weight = (1.0f - fabs(rx)) * (1.0f - fabs(ry));
        Vec3f rd = Vec3f(x, y, 0) + jitter - Vec3f(0, 0, -z);
        rd.normalize();
        if (lensRadius > 0) {
            Point2f pLens = Point2f(lensRadius, lensRadius) * ConcentricSampleDisk(sampler.uniformFloat());
            Float ft = focalDistance / rd.z();
            auto pFocus = ro + ft * rd;
            ro = Vec3f(pLens.x(), pLens.y(), 0);
            rd = (pFocus - ro).normalized();
        }
        ro.w() = 1;
        ro = rotationMatrix.mult(ro);
        ro += viewpot;


        rd.w() = 1;
        rd = rotationMatrix.mult(rd);
        rd.normalize();
        *ray = Ray{ro, rd};
        return 1;
    }

    Float PerspectiveCamera::generateRayDifferential(
            Sampler &sampler, const Point2i &raster,
            RayDifferential *ray, Float *weight) {
        throw NotImplemented();
        return 0;
    }

}