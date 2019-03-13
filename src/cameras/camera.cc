//
// Created by Shiina Miyuki on 2019/3/3.
//

#include "camera.h"
#include "samplers/sampler.h"
#include "math/sampling.h"
#include <lights/light.h>
#include <core/scatteringevent.h>

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
        ro = cameraToWorld(ro);
        ro += viewpot;

        rd = cameraToWorld(rd).normalized();
        *ray = Ray{ro, rd};
        Point2i pRaster;
        rasterize(ro + 1 * rd, &pRaster);
        Assert(abs(pRaster.x() - raster.x()) <= 1);
        Assert(abs(pRaster.y() - raster.y()) <= 1);
        return 1;
    }

    Float PerspectiveCamera::generateRayDifferential(
            Sampler &sampler, const Point2i &raster,
            RayDifferential *ray, Float *weight) {
        throw NotImplemented();
        return 0;
    }

    Spectrum PerspectiveCamera::We(const Ray &ray, Point2i *pRaster) const {
        auto rd = worldToCamera(ray.d).normalized();
        auto cosT = Vec3f::dot(rd, Vec3f(0, 0, 1));
        if (cosT < 0) {
            return {};
        }
        auto z0 = (Float) (2.0 / std::tan(fov / 2));
        Point2f raster(rd.x() / rd.z() * z0, rd.y() / rd.z() * z0);
        auto w = (float) dimension.x() / dimension.y();
        // check if out of bound
        if (fabs(raster.x()) > w + 0.001f || fabs(raster.y()) > 1 + 0.001f) {
            return {};
        }

        int x = lroundf(((-raster.x() + w) / (2 * w)) * dimension.x());
        int y = lroundf((1 - (raster.y() + 1.0f) / 2.0f) * dimension.y());
        *pRaster = {x, y};
        Float lensArea = lensRadius <= 0 ? 1 : PI * lensRadius * lensRadius;
        auto cosT2 = cosT * cosT;
        Assert(A > 0);
        auto weight = 1.0f / (A * lensArea * cosT2 * cosT2);
        CHECK(!std::isinf(weight));
        return {weight, weight, weight};
    }

    Spectrum PerspectiveCamera::sampleWi(const ScatteringEvent &event, const Point2f &u, Vec3f *wi, Float *pdf,
                                         Point2i *pRaster, VisibilityTester *tester) {
        Point2f pLens = Point2f(lensRadius, lensRadius) * ConcentricSampleDisk(u);
        auto pLensWorld = cameraToWorld(Vec3f(pLens.x(), pLens.y(), 0)) + viewpot;
        tester->primId = event.getIntersection()->primId;
        tester->geomId = event.getIntersection()->geomId;
        *wi = pLensWorld - event.getIntersection()->ref;
        auto dist = wi->length();
        *wi /= dist;
        tester->shadowRay = Ray(pLensWorld, -1 * *wi);
        Float lensArea = lensRadius <= 0 ? 1 : PI * lensRadius * lensRadius;
        Vec3f lensNormal = cameraToWorld(Vec3f(0, 0, 1)).normalized();
        *pdf = (dist * dist) / (Vec3f::absDot(*wi, lensNormal) * lensArea);
        return We(event.spawnRay(-1 * *wi), pRaster);
    }

    bool PerspectiveCamera::rasterize(const Vec3f &p, Point2i *rasterPos) const {

        auto dir = worldToCamera((p - viewpot).normalized()).normalized();
        auto cosT = Vec3f::dot(dir, Vec3f(0, 0, 1));
        if (cosT < 0) {
            return false;
        }
        auto z0 = (Float) (2.0 / tan(fov / 2));
        Point2f raster(dir.x() / dir.z() * z0, dir.y() / dir.z() * z0);
        auto w = (float) dimension.x() / dimension.y();
        if (fabs(raster.x()) > w + 0.01f || fabs(raster.y()) > 1.01f) {
            return false;
        }
        int x = lroundf(((-raster.x() + w) / (2 * w)) * dimension.x());
        int y = lroundf((1 - (raster.y() + 1.0f) / 2.0f) * dimension.y());
        *rasterPos = {x, y};
        return true;

    }

    void PerspectiveCamera::preprocess() {
        Camera::preprocess();
        auto z0 = (Float) (2.0 / tan(fov / 2));
        A = 2 * ((2.0f * dimension.x()) / dimension.y()) / (z0 * z0);
    }

    void PerspectiveCamera::pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const {
        auto rd = worldToCamera(ray.d);
        auto cosT = Vec3f::dot(rd, Vec3f(0, 0, 1));
        if (cosT < 0) {
            *pdfPos = *pdfDir = 0;
            return;
        }
        auto z0 = (Float) (2.0 / std::tan(fov / 2));
        Point2f raster(rd.x() / rd.z() * z0, rd.y() / rd.z() * z0);
        // check if out of bound
        if (fabs(raster.x()) > (float) dimension.x() / dimension.y() || fabs(raster.y()) > 1.0f) {
            *pdfPos = *pdfDir = 0;
            return;
        }
        Float lensArea = lensRadius <= 0 ? 1 : PI * lensRadius * lensRadius;
        *pdfPos = 1 / lensArea;
        auto cosT2 = cosT * cosT;
        *pdfDir = 1.0f / (A * lensArea * cosT * cosT2);
    }

}