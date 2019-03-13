//
// Created by Shiina Miyuki on 2019/3/3.
//

#ifndef MIYUKI_CAMERA_H
#define MIYUKI_CAMERA_H

#include "miyuki.h"
#include "core/geometry.h"
#include "core/ray.h"
#include "math/transform.h"
#include "core/parameter.h"

namespace Miyuki {
    class Scene;

    class Sampler;

    struct ScatteringEvent;
    struct VisibilityTester;

    class Camera {
        friend class Scene;

    protected:
        Vec3f viewpot;
        // euler angle;
        Vec3f direction;
        Matrix4x4 rotationMatrix, invMatrix;

        Point2i dimension;
    public:
        Camera(const Point2i &dim) : dimension(dim) {}

        void moveTo(const Vec3f &v);

        void move(const Vec3f &v);

        void rotate(const Vec3f &v);

        void rotateTo(const Vec3f &v);

        void computeTransformMatrix();

        virtual Float generateRay(Sampler &sampler, const Point2i &raster, Ray *ray, Float *weight) = 0;

        virtual Float
        generateRayDifferential(Sampler &sampler, const Point2i &raster, RayDifferential *ray, Float *weight) = 0;

        virtual void preprocess() { computeTransformMatrix(); }

        virtual Spectrum We(const Ray &ray, Point2i *raster) const = 0;

        virtual Spectrum
        sampleWi(const ScatteringEvent &event, const Point2f &u, Vec3f *wi, Float *pdf, Point2i *pRaster,
                 VisibilityTester *) = 0;

        virtual void pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const = 0;

        Vec3f cameraToWorld(Vec3f w) const {
            w.w() = 1;
            w = rotationMatrix.mult(w);
            return w;
        }

        Vec3f worldToCamera(Vec3f w) const {
            w.w() = 1;
            w = invMatrix.mult(w);
            return w;
        }

        virtual bool rasterize(const Vec3f &p, Point2i *rasterPos) const = 0;

    };

    class PerspectiveCamera : public Camera {
        friend class Scene;

        Float fov;
        Float lensRadius;
        Float focalDistance;
        Float A;
    public:
        PerspectiveCamera(const Point2i &dim, Float fov, Float lensRadius = 0, Float focalDistance = 0)
                : Camera(dim),
                  fov(fov), lensRadius(lensRadius), focalDistance(focalDistance) {}

        Float generateRay(Sampler &sampler, const Point2i &raster, Ray *ray, Float *weight) override;

        Float generateRayDifferential(Sampler &sampler, const Point2i &raster,
                                      RayDifferential *ray, Float *weight) override;

        Spectrum We(const Ray &ray, Point2i *raster) const override;

        void preprocess() override;

        Spectrum
        sampleWi(const ScatteringEvent &event, const Point2f &u, Vec3f *wi, Float *pdf, Point2i *pRaster,
                 VisibilityTester *tester) override;

        void pdfWe(const Ray &ray, Float *pdfPos, Float *pdfDir) const override;

        bool rasterize(const Vec3f &p, Point2i *rasterPos) const override;
    };

    std::unique_ptr<PerspectiveCamera> CreatePerspectiveCamera(const ParameterSet &);
}
#endif //MIYUKI_CAMERA_H
