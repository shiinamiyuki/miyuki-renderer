//
// Created by Shiina Miyuki on 2019/2/13.
//

#include "camera.h"
#include "../core/ray.h"
#include "../samplers/sampler.h"

using namespace Miyuki;

Point2f concentricSampleDisk(const Point2f &u) {

    Point2f uOffset = Point2f{2.0f, 2.0f} * u - Point2f(1, 1);

    if (uOffset.x() == 0 && uOffset.y() == 0)
        return Point2f(0, 0);

    Float theta, r;
    if (std::abs(uOffset.x()) > std::abs(uOffset.y())) {
        r = uOffset.x();
        theta = PI / 4.0f * (uOffset.y() / uOffset.x());
    } else {
        r = uOffset.y();
        theta = PI / 2.0f - PI / 4.0f * (uOffset.x() / uOffset.y());
    }
    return Point2f{r, r} * Point2f(std::cos(theta), std::sin(theta));

}

Float Camera::generatePrimaryRay(Sampler &sampler, const Point2i &raster, Ray *ray, Float *weight) {
    Float x = -(2 * (Float) raster.x() / filmDimension.x() - 1) * static_cast<Float>(filmDimension.x()) /
              filmDimension.y();
    Float y = 2 * (1 - (Float) raster.y() / filmDimension.y()) - 1;
    Float dx = 2.0f / filmDimension.y() / 2, dy = 2.0f / filmDimension.y() / 2;
    Vec3f ro(0, 0, 0);
    auto z = (Float) (2.0 / tan(fov / 2));
    Float rx = 2 * sampler.nextFloat() - 1;
    Float ry = 2 * sampler.nextFloat() - 1;
    Vec3f jitter = Vec3f(dx * rx, dy * ry, 0);
    *weight = (1.0f - fabs(rx)) * (1.0f - fabs(ry));
    Vec3f rd = Vec3f(x, y, 0) + jitter - Vec3f(0, 0, -z);
    rd.normalize();
    if (lensRadius > 0) {
        Point2f pLens = Point2f(lensRadius, lensRadius) * concentricSampleDisk(sampler.randFloat());
        Float ft = focalDistance / rd.z();
        auto pFocus = ro + ft * rd;
        ro = Vec3f(pLens.x(), pLens.y(), 0);
        rd = (pFocus - ro).normalized();
    }
    ro.w() = 1;
    ro = matrix.mult(ro);
    ro += viewpoint;


    rd.w() = 1;
    rd = matrix.mult(rd);
    rd.normalize();
    *ray = Ray{ro, rd};
    return 1;
}

void Camera::lookAt(const Vec3f &pos) {
    assert(false);
}

void Camera::initTransformMatrix() {
//    matrix = Matrix4x4::rotation(Vec3f(1, 0, 0), -direction.y());
//    matrix = matrix.mult(Matrix4x4::rotation(Vec3f(0, 1, 0), direction.x()));
//    matrix = matrix.mult(Matrix4x4::rotation(Vec3f(0, 0, 1), direction.z()));
    matrix = Matrix4x4::rotation(Vec3f(0, 0, 1), direction.z());
    matrix = matrix.mult(Matrix4x4::rotation(Vec3f(0, 1, 0), direction.x()));
    matrix = matrix.mult(Matrix4x4::rotation(Vec3f(1, 0, 0), -direction.y()));
    // matrix = matrix.mult(Matrix4x4::translation(viewpoint));
    Matrix4x4::inverse(matrix, invMatrix);
    /* Now the perspective matrix
     z0 = 2 / tan(fov/2)
     The image plane is from (-w/h, -1, z0) to (w/h, 1, z0)
     For a point p = (x, y, z)
     The projected point p' = (z0 * x/z, z0 *y/z, z0):
     \frac{x'}{x} = \frac{z0}{z}
     \frac{y'}{y} = \frac{z0}{z}
     The homogeneous coordinate is (z0 *x, z0*y, z0 *z ,z)
     */
    auto z0 = (Float) (2.0 / tan(fov / 2));
    Float m[4][4] = {
            {z0, 0,  0,  0},
            {0,  z0, 0,  0},
            {0,  0,  z0, 0},
            {0,  0,  1,  0}
    };
    perspectiveMatrix = Matrix4x4(m);
    normal = Vec3f{0, 0, 1};
    normal.w() = 1;
    normal = matrix.mult(normal);
    normal.normalize();
    // 1 / z0^2 = A / A'
    // A = A' / z0^2
    A = 2 * ((2.0f * filmDimension.x()) / filmDimension.y()) / (z0 * z0);
}

void Camera::pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const {
    auto rd = ray.d;
    rd.w() = 1;
    rd = invMatrix.mult(rd);
    rd.normalize();
    auto cosT = Vec3f::dot(rd, Vec3f(0, 0, 1));
    if (cosT < 0) {
        *pdfPos = *pdfDir = 0;
        return;
    }
    auto z0 = (Float) (2.0 / tan(fov / 2));
    Point2f raster(rd.x() / rd.z() * z0, rd.y() / rd.z() * z0);
    // check if out of bound
    if (fabs(raster.x()) > (float) filmDimension.x() / filmDimension.y() || fabs(raster.y()) > 1.0f) {
        *pdfPos = *pdfDir = 0;
        return;
    }
    Float lensArea = 1;
    *pdfPos = 1 / lensArea;
    auto cosT2 = cosT * cosT;
    *pdfDir = 1.0f / (A * cosT * cosT2);

}

Spectrum Camera::We(const Ray &ray) {
    auto rd = ray.d;
    rd.w() = 1;
    rd = invMatrix.mult(rd);
    rd.normalize();
    auto cosT = Vec3f::dot(rd, Vec3f(0, 0, 1));
    if (cosT < 0) {
        return {};
    }
    auto z0 = (Float) (2.0 / tan(fov / 2));
    Point2f raster(rd.x() / rd.z() * z0, rd.y() / rd.z() * z0);
    // check if out of bound
    if (fabs(raster.x()) > (float) filmDimension.x() / filmDimension.y() || fabs(raster.y()) > 1.0f) {
        return {};
    }
    auto cosT2 = cosT * cosT;
    auto w = 1.0f / (A * cosT2 * cosT2);
    return {w, w, w};
}

bool Camera::rasterize(const Vec3f &p, Point2i *rasterPos) const {
    auto dir = (p - viewpoint).normalized();
    dir.w() = 1;
    dir = invMatrix.mult(dir);
    dir.normalize();
    auto cosT = Vec3f::dot(dir, Vec3f(0, 0, 1));
    if (cosT < 0) {
        return false;
    }
    auto z0 = (Float) (2.0 / tan(fov / 2));
    Point2f raster(dir.x() / dir.z() * z0, dir.y() / dir.z() * z0);
    auto w = (float) filmDimension.x() / filmDimension.y();
    if (fabs(raster.x()) > w + 0.01f || fabs(raster.y()) > 1.01f) {
        return false;
    }
    int x = lroundf(((-raster.x() + w) / (2 * w)) * filmDimension.x());
    int y = lroundf((1 - (raster.y() + 1.0f) / 2.0f) * filmDimension.y());
    *rasterPos = {x, y};
    return true;
}

