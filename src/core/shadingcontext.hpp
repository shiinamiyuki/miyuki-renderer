//
// Created by Shiina Miyuki on 2019/4/8.
//

#ifndef MIYUKI_SHADINGCONTEXT_HPP
#define MIYUKI_SHADINGCONTEXT_HPP

#include <core/spectrum.h>
#include <math/variance.hpp>

namespace Miyuki {
    struct ShadingContextElement {
        Spectrum value;
        Variance<Spectrum> variance;
        Float weightSum = 0;
        bool valid = false;

        ShadingContextElement() {}

        ShadingContextElement(const Spectrum &value, Float weight = 1)
                : value(value * weight), weightSum(weight), valid(true) {}

        void addSample(const Spectrum &value, Float weight = 1) {
            auto v = value * weight;
            this->value += value;
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
        ShadingContextElement albedo;
        ShadingContextElement depth;
        ShadingContextElement normal;

        void combineSamples(const ShadingContext &ctx) {
            color.combineSample(ctx.color);
            albedo.combineSample(ctx.albedo);
            depth.combineSample(ctx.depth);
            normal.combineSample(ctx.normal);
        }
    };
}
#endif //MIYUKI_SHADINGCONTEXT_HPP
