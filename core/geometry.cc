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

Miyuki::Vec3f
Miyuki::GGXImportanceSampling(Float roughness, const Miyuki::Vec3f &norm, Miyuki::Float u1, Miyuki::Float u2) {
    auto tm = atan(roughness * sqrt(u1 / (1 - u1)));
    auto phim = 2 * M_PI * u2;
    auto x = norm.x();
    Vec3f u = Vec3f::cross((fabs(x) > 0.1)
                           ? Vec3f(0, 1, 0)
                           : Vec3f(1, 0, 0), norm);
    Vec3f v = Vec3f::cross(norm, u);
    Vec3f r = u * cos(phim) * sin(tm) + v * sin(phim) * sin(tm) + norm * cos(tm);
    return r;
}

Vec3f Miyuki::pointOnTriangle(const Vec3f &v1, const Vec3f &v2, const Vec3f &v3, Float u1, Float u2) {
    return (1 - u1 - u2) * v1 + u1 * v2 + u2 * v3;//v1 + u1*(v2 - v1) + u2 * (v3 - v1);
}

// https://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
Float Miyuki::GGXDistribution(const Vec3f &m, const Vec3f &n, float alpha_g) {
    alpha_g *= alpha_g;
    float d = Vec3f::dot(m, n);
    if (d <= 0)return 0;
    return (Float) std::max(0.0, alpha_g / (M_PI * pow(d * d * (alpha_g - 1) + 1, 2)));
}
