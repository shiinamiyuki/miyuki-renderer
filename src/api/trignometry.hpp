#pragma once

#include <api/math.hpp>

namespace miyuki {

    inline Float CosTheta(const Vec3f &w) { return w.y; }
    inline Float AbsCosTheta(const Vec3f &w) { return std::abs(CosTheta(w)); }
    inline Float Cos2Theta(const Vec3f &w) { return w.y * w.y; }
    inline Float Sin2Theta(const Vec3f &w) { return 1 - Cos2Theta(w); }
    inline Float SinTheta(const Vec3f &w) { return std::sqrt(std::fmax(0.0f, Sin2Theta(w))); }
    inline Float Tan2Theta(const Vec3f &w) { return Sin2Theta(w) / Cos2Theta(w); }
    inline Float TanTheta(const Vec3f &w) { return std::sqrt(std::fmax(0.0f, Tan2Theta(w))); }
    inline bool SameHemisphere(const Vec3f &wo, const Vec3f &wi) { return wo.y * wi.y >= 0; }

} // namespace miyuki