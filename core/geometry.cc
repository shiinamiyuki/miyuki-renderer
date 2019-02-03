//
// Created by Shiina Miyuki on 2019/1/12.
//

#include "geometry.h"

using namespace Miyuki;

Miyuki::Vec3f
Miyuki::cosineWeightedHemisphereSampling(const Miyuki::Vec3f &normal, Miyuki::Float u1, Miyuki::Float u2) {
    double r1 = 2 * M_PI * u1, r2 = u2, r2s = sqrt(r2);
    const auto &w = normal;
    auto u = Vec3f::cross((abs(w.x()) > 0.1) ? Vec3f{0, 1, 0} : Vec3f{1, 0, 0}, w);
    auto v = Vec3f::cross(w, u);
    auto r = Vec3f(u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2));
    r.normalize();
    return r;
}

Vec3f Miyuki::cosineWeightedHemisphereSampling(const Point2f &u) {
    double r1 = 2 * M_PI * u.x(), r2 = u.y(), r2s = sqrt(r2);
    return Vec3f(cos(r1) * r2s, sin(r1) * r2s, sqrt(1 - r2));
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
Miyuki::GGXImportanceSampling(Float roughness, const Miyuki::Vec3f &normal, Miyuki::Float u1, Miyuki::Float u2) {
    auto tm = atan(roughness * sqrt(u1 / (1 - u1)));
    auto phim = 2 * M_PI * u2;
    auto x = normal.x();
    Vec3f u = Vec3f::cross((fabs(x) > 0.1)
                           ? Vec3f(0, 1, 0)
                           : Vec3f(1, 0, 0), normal);
    Vec3f v = Vec3f::cross(normal, u);
    Vec3f r = u * cos(phim) * sin(tm) + v * sin(phim) * sin(tm) + normal * cos(tm);
    r.normalize();
    return r;
}


// https://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
Float Miyuki::GGXDistribution(const Vec3f &m, const Vec3f &n, float alpha_g) {
    alpha_g *= alpha_g;
    float d = Vec3f::dot(m, n);
    if (d <= 0)return 0;
    return (Float) std::max(0.0, alpha_g / (M_PI * pow(d * d * (alpha_g - 1) + 1, 2)));
}

