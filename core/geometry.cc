//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "geometry.h"

using namespace Miyuki;

Miyuki::Vec3f Miyuki::cosineWeightedHemisphereSampling(const Miyuki::Vec3f &norm, Miyuki::Float u1, Miyuki::Float u2) {
    double r1 = 2 * M_PI * u1, r2 = u2, r2s = sqrt(r2);
    const auto &w = norm;
    auto u = Vec3f::cross((abs(w.x()) > 0.1) ? Vec3f{0, 1, 0} : Vec3f{1, 0, 0}, w);
    auto v = Vec3f::cross(w, u);
    auto r = Vec3f(u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2));
    return r;
}

Miyuki::Vec3f Miyuki::sphereSampling(Miyuki::Float u1, Miyuki::Float u2) {
    auto z = 1.0 - 2.0 * u1;
    auto r = sqrt(std::max(0.0, 1.0 - z * z));
    auto phi = 2.0 * M_PI * u2;
    auto x = r * cos(phi);
    auto y = r * sin(phi);
    return Vec3f(x, y, z);
}

Miyuki::Vec3f Miyuki::GGXImportanceSampling(const Miyuki::Vec3f &norm, Miyuki::Float u1, Miyuki::Float u2) {
    return Miyuki::Vec3f();
}
