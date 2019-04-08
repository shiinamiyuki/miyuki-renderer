//
// Created by Shiina Miyuki on 2019/4/8.
//

#ifndef MIYUKI_SHADINGCONTEXT_HPP
#define MIYUKI_SHADINGCONTEXT_HPP

#include <core/spectrum.h>

namespace Miyuki {
    struct ShadingContextElement {
        Spectrum value;
        Float weightSum = 0;
        bool valid = false;

        ShadingContextElement() {}

        ShadingContextElement(const Spectrum& value, Float weight = 1)
                : value(value * weight), weightSum(weight), valid(true) {}

        void addSample(const Spectrum &value, Float weight = 1) {
            this->value += value * weight;
            weightSum += -weight;
        }

        void combineSample(const ShadingContextElement &element) {
            if (element.valid) {
                value += element.value;
                weightSum += element.weightSum;
                valid = true;
            }
        }

        Spectrum eval() const {
            if (!valid || weightSum == 0) {
                return Spectrum{};
            }
            return removeNaNs(value / weightSum);
        }
    };

    struct ShadingContext {
        ShadingContextElement color;
        ShadingContextElement direct;
        ShadingContextElement indirect;
        ShadingContextElement albedo;
        ShadingContextElement depth;
        ShadingContextElement normal;

        void combineSamples(const ShadingContext &ctx) {
            color.combineSample(ctx.color);
            direct.combineSample(ctx.direct);
            indirect.combineSample(ctx.indirect);
            albedo.combineSample(ctx.albedo);
            depth.combineSample(ctx.depth);
            normal.combineSample(ctx.normal);
        }
    };
}
#endif //MIYUKI_SHADINGCONTEXT_HPP
