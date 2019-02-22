//
// Created by Shiina Miyuki on 2019/2/8.
//

#ifndef MIYUKI_SCATTERINGEVENT_H
#define MIYUKI_SCATTERINGEVENT_H

#include "../utils/util.h"
#include "../core/spectrum.h"
#include "intersection.h"

namespace Miyuki {
    class Intersection;

    enum class BSDFType;
    struct IntersectionInfo;
    struct Ray;

    class Sampler;

    enum class TransportMode {
        radiance,
        importance
    };

    class ScatteringEvent {
        const IntersectionInfo *info;

        void computeLocalCoordinates();

    public:
        /* wo and wi should be in the positive-y hemisphere for reflection
         * */
        Vec3f Ns;
        Vec3f localX, localZ;
        Vec3f wiW;
        Vec3f woW;
        Vec3f wo, wi;
        Float pdf;
        BSDFType sampledType;
        BSDFType flags;
        Point2f u;

        ScatteringEvent() : info(nullptr) {}

        ScatteringEvent(const IntersectionInfo *info, Sampler *sampler);

        Vec3f worldToLocal(const Vec3f &v) const; // transform according to shading normal

        Vec3f localToWorld(const Vec3f &v) const;

        const Point2f &uv() const;

        const Vec3f &hitPoint() const;

        const Vec3f &Ng() const;

        const IntersectionInfo *getIntersectionInfo() const;

        Ray spawnRay(const Vec3f &wi) const {
            return {hitPoint(), wi};
        }

        Ray spawnRayLocal(const Vec3f &wi) const {
            return {hitPoint(), localToWorld(wi)};
        }

        Spectrum Le(const Vec3f &wo) const;

        void setWi(const Vec3f &wi) {
            this->wi = wi;
            wiW = localToWorld(wi);
        }
    };
}

#endif //MIYUKI_SCATTERINGEVENT_H
