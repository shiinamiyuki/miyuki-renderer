//
// Created by Shiina Miyuki on 2019/1/17.
//

#ifndef MIYUKI_LIGHT_H
#define MIYUKI_LIGHT_H

#include "util.h"
#include "spectrum.h"

namespace Miyuki {
    class Light {
    public:
        enum class Type {
            deltaPosition = 1,
            deltaDirection = 2,
            area = 4,
            infnite = 8
        } type;

        Light() = delete;

        virtual Spectrum sampleLe() = 0;

        virtual Spectrum sampleLi() = 0;

        virtual Spectrum power() = 0;

        bool isDeltaLight() const {
            return ((int) type & (int) Type::deltaDirection)
                   || ((int) type & (int) Type::deltaPosition);
        }
    };
}
#endif //MIYUKI_LIGHT_H
