//
// Created by Shiina Miyuki on 2019/4/1.
//

#ifndef MIYUKI_FILTER_H
#define MIYUKI_FILTER_H

#include <core/geometry.h>

namespace Miyuki {
    class Filter {
    public:
        Point2f radius;
        Filter(const Point2f &radius) : radius(radius) {}

        virtual Float eval(const Point2f &) const = 0;
    };

    class BoxFilter : public Filter {
    public:
        BoxFilter(const Point2f &radius) : Filter(radius){}
        Float eval(const Point2f &p) const override {
            return 1.0f;
        }
    };

    class TriangleFilter : public Filter {
    public:
        TriangleFilter(const Point2f &radius) : Filter(radius){}
        Float eval(const Point2f &p) const override {
            return std::max((Float) 0, radius.x() - std::abs(p.x())) *
                   std::max((Float) 0, radius.y() - std::abs(p.y()));
        }
    };

    class MitchellFilter : public Filter {
        Float B, C;
        Point2f invRadius;
    public:
        MitchellFilter(const Point2f &radius, Float B, Float C) : Filter(radius),B(B),C(C) {
            invRadius = Point2f(1, 1) / radius;
        }

        Float Mitchell1D(Float x) const {
            x = std::abs(2 * x);
            if (x > 1)
                return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x +
                        (-12 * B - 48 * C) * x + (8 * B + 24 * C)) * (1.f / 6.f);
            else
                return ((12 - 9 * B - 6 * C) * x * x * x +
                        (-18 + 12 * B + 6 * C) * x * x +
                        (6 - 2 * B)) * (1.f / 6.f);
        }

        Float eval(const Point2f &p) const override {
            return Mitchell1D(p.x() * invRadius.x()) * Mitchell1D(p.y() * invRadius.y());
        }
    };
}
#endif //MIYUKI_FILTER_H
