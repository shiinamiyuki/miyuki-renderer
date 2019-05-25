//
// Created by Shiina Miyuki on 2019/3/7.
//

#ifndef MIYUKI_FUNC_H
#define MIYUKI_FUNC_H

#include "miyuki.h"

namespace Miyuki {
    inline Float PowerHeuristics(Float pdf1, Float pdf2) {
        Float p1 = pdf1 * pdf1;
        Float p2 = pdf2 * pdf2;
        return p1 / (p1 + p2);
    }

    inline Float ErfInv(Float x) {
        Float w, p;
        x = clamp(x, -.99999f, .99999f);
        w = -std::log((1 - x) * (1 + x));
        if (w < 5) {
            w = w - 2.5f;
            p = 2.81022636e-08f;
            p = 3.43273939e-07f + p * w;
            p = -3.5233877e-06f + p * w;
            p = -4.39150654e-06f + p * w;
            p = 0.00021858087f + p * w;
            p = -0.00125372503f + p * w;
            p = -0.00417768164f + p * w;
            p = 0.246640727f + p * w;
            p = 1.50140941f + p * w;
        } else {
            w = std::sqrt(w) - 3;
            p = -0.000200214257f;
            p = 0.000100950558f + p * w;
            p = 0.00134934322f + p * w;
            p = -0.00367342844f + p * w;
            p = 0.00573950773f + p * w;
            p = -0.0076224613f + p * w;
            p = 0.00943887047f + p * w;
            p = 1.00167406f + p * w;
            p = 2.83297682f + p * w;
        }
        return p * x;
    }

    inline Float Erf(Float x) {
        // constants
        Float a1 = 0.254829592f;
        Float a2 = -0.284496736f;
        Float a3 = 1.421413741f;
        Float a4 = -1.453152027f;
        Float a5 = 1.061405429f;
        Float p = 0.3275911f;

        // Save the sign of x
        int sign = 1;
        if (x < 0) sign = -1;
        x = std::abs(x);

        // A&S formula 7.1.26
        Float t = 1 / (1 + p * x);
        Float y =
                1 -
                (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);

        return sign * y;
    }

    template<typename T>
    inline T RadiansToDegrees(const T &x) {
        return x * INVPI * 180.0;
    }

    template<typename T>
    inline T DegreesToRadians(const T &x) {
        return x / 180.0 * PI;
    }

    inline bool SolveLinearSystem2x2(const Float A[2][2], const Float B[2], Float *x1, Float *x2) {
        Float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
        if (std::abs(det) < 1e-10f)
            return false;
        *x1 = (A[1][1] * B[0] - A[0][1] * B[1]) / det;
        *x2 = (A[0][0] * B[1] - A[1][0] * B[0]) / det;
        return !(std::isnan(*x1) || std::isnan(*x2));
    }
}
#endif //MIYUKI_FUNC_H
