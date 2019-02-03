//
// Created by Shiina Miyuki on 2019/1/17.
//

#ifndef MIYUKI_LIGHT_H
#define MIYUKI_LIGHT_H

#include "util.h"
#include "spectrum.h"
#include "interaction.h"
#include "ray.h"
#include "mesh.h"

namespace Miyuki {
    struct Interaction;

    class TriangularMesh;

    class MeshInstance;

    struct Primitive;

    class Light;

    class Scene;

    struct VisibilityTester {
        int targetGeomID;
        int targetPrimID;
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
        // virtual Spectrum sampleLe(Float *pdfPos, Float *pdfDir) const = 0;

        // u used for sampling a point on light, wi is the sampled direction, from L to x
        virtual Spectrum sampleLi(const Point2f &u,
                                  const Interaction &interaction,
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
            return ((int) type & (int) Type::deltaDirection)
                   || ((int) type & (int) Type::deltaPosition);
        }

        void scalePower(Float k) { ka *= k; }
    };
    namespace Mesh { struct Primitive; }
    class AreaLight : public Light {
        Ref<const Primitive> primitive;
        Float area;
    public:
        AreaLight(const Primitive &, const Spectrum &ka);

        Spectrum sampleLi(const Point2f &u,
                          const Interaction &interaction,
                          Vec3f *wi,
                          Float *pdf,
                          VisibilityTester *) const override;

        Float power() const override;

        Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                          Float *pdfDir) const override;
    };

    class PointLight : public Light {
    protected:
        Vec3f position;
    public:
        PointLight(const Spectrum &_ka, const Vec3f &pos) : Light(_ka), position(pos) {
            type = Type::deltaPosition;
        }

        Spectrum sampleLi(const Point2f &u, const Interaction &interaction, Vec3f *wi, Float *pdf,
                          VisibilityTester *tester) const override;

        Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Ray *ray, Vec3f *normal, Float *pdfPos,
                          Float *pdfDir) const override;
    };
}
#endif //MIYUKI_LIGHT_H
