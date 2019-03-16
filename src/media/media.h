//
// Created by Shiina Miyuki on 2019/3/16.
//

#ifndef MIYUKI_MEDIA_H
#define MIYUKI_MEDIA_H

#include <core/scatteringevent.h>

namespace Miyuki {
    class PhaseFunction {
    public:
        // eval the phase function
        virtual Float p(const Vec3f &wo, const Vec3f &wi) const = 0;

        virtual Float sampleP(const Vec3f &wo, const Vec3f *wi, const Point2i &u) const = 0;
    };
}
#endif //MIYUKI_MEDIA_H
