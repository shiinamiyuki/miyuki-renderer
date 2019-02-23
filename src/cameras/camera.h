//
// Created by Shiina Miyuki on 2019/2/13.
//

#ifndef MIYUKI_CAMERA_H
#define MIYUKI_CAMERA_H

#include "../math/geometry.h"
#include "../math/transform.h"
#include "../core/spectrum.h"

namespace Miyuki {
    class Sampler;

    struct Ray;

    class Camera {
        Matrix4x4 matrix, invMatrix;
        Matrix4x4 perspectiveMatrix;
        Float A;
    public:
        Vec3f viewpoint;
        Vec3f direction;
        double fov;
        Point2i filmDimension;
        Vec3f normal;
        void moveTo(const Vec3f &pos) { viewpoint = pos; }

        void rotateTo(const Vec3f &dir) {
            direction = dir;
        }

        void rotate(const Vec3f &dir) {
            direction += dir;
        }

        void lookAt(const Vec3f &pos);

        Camera(const Vec3f &v = Vec3f(0, 0, 0), const Vec3f &d = Vec3f(0, 0, 0))
                : viewpoint(v), direction(d), fov(M_PI / 2) {}

        void initTransformMatrix();

        Float generatePrimaryRay(Sampler &sampler, const Point2i &raster, Ray *ray);

        void pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const;
        Spectrum We(const Ray& ray);
        bool rasterize(const Vec3f &p, Point2i *raster) const;

    };
}
#endif //MIYUKI_CAMERA_H
