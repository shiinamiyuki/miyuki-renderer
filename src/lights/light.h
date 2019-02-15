//
// Created by Shiina Miyuki on 2019/1/17.
//

#ifndef MIYUKI_LIGHT_H
#define MIYUKI_LIGHT_H

#include "../utils/util.h"
#include "../core/spectrum.h"
#include "../core/scatteringevent.h"
#include "../core/ray.h"
#include "../core/mesh.h"
#include "../core/intersection.h"

namespace Miyuki {
    struct IntersectionInfo;

    class TriangularMesh;

    class MeshInstance;

    struct Primitive;

    class Light;

    class Scene;

    class ScatteringEvent;

    struct VisibilityTester {
        int32_t targetGeomID;
        int32_t targetPrimID;
        Ray shadowRay;

        VisibilityTester()
                : shadowRay(Vec3f(), Vec3f()), targetGeomID(-1), targetPrimID(-1) {}

        bool visible(const Scene &) const;
    };


    class Light {
    protected:
        Spectrum ka;
    public:
        enum class Type {
            deltaPosition = 1,
            deltaDirection = 2,
            area = 4,
            infinite = 8
        } type;

        Light() : ka() {}

        explicit Light(const Spectrum &_ka) : ka(_ka) {}

        virtual Spectrum sampleLi(const Point2f &u,
                                  const IntersectionInfo &info,
                                  Vec3f *wi,
                                  Float *pdf,
                                  VisibilityTester *) const = 0;

        virtual Spectrum sampleLe(const Point2f &u1,
                                  const Point2f &u2,
                                  Ray *ray,
                                  Vec3f *normal,
                                  Float *pdfPos,
                                  Float *pdfDir) const = 0;

        virtual Float power() const { return ka.max(); }

        bool isDeltaLight() const {
            return ((int32_t) type & (int32_t) Type::deltaDirection)
                   || ((int32_t) type & (int32_t) Type::deltaPosition);
        }

        virtual Float pdfLi(const IntersectionInfo &, const Vec3f &wi) const = 0;

        virtual void pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const = 0;

        virtual const Primitive *getPrimitive() const { return nullptr; }

        void scalePower(Float k) { ka *= k; }
    };

    class AreaLight : public Light {
        Ref<const Primitive> primitive;
        Float area;
    public:
        AreaLight(const Primitive &, const Spectrum &ka);

        Spectrum sampleLi(const Point2f &u,
                          const IntersectionInfo &info,
                          Vec3f *wi,
                          Float *pdf,
                          VisibilityTester *) const override;

        Float power() const override;

        Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                          Float *pdfDir) const override;

        Float pdfLi(const IntersectionInfo &info, const Vec3f &wi) const override;

        const Primitive *getPrimitive() const override {
            return primitive.raw();
        }

        void pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const override;

    };

    class PointLight : public Light {
    protected:
        Vec3f position;
    public:
        PointLight(const Spectrum &_ka, const Vec3f &pos) : Light(_ka), position(pos) {
            type = Type::deltaPosition;
        }

        Spectrum sampleLi(const Point2f &u, const IntersectionInfo &info, Vec3f *wi, Float *pdf,
                          VisibilityTester *tester) const override;

        Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                          Float *pdfDir) const override;

        Float pdfLi(const IntersectionInfo &info, const Vec3f &wi) const override;

        void pdfLe(const Ray &ray, const Vec3f &normal, Float *pdfPos, Float *pdfDir) const override;
    };
}
#endif //MIYUKI_LIGHT_H
