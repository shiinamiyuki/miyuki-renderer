//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_TRANSFORM_H
#define MIYUKI_TRANSFORM_H

#include "miyuki.h"
#include "core/geometry.h"

namespace Miyuki {
    struct Matrix4x4 {
        Vec3f m[4];

        Matrix4x4();

        Matrix4x4(const Vec3f &r1, const Vec3f &r2, const Vec3f &r3, const Vec3f &r4);

        Matrix4x4(Float _m[4][4]);

        Matrix4x4 &operator+=(const Matrix4x4 &rhs);

        Matrix4x4 &operator-=(const Matrix4x4 &rhs);

        Matrix4x4 &operator*=(const Matrix4x4 &rhs);

        Matrix4x4 &operator/=(const Matrix4x4 &rhs);

        Matrix4x4 operator+(const Matrix4x4 &rhs) const;

        Matrix4x4 operator-(const Matrix4x4 &rhs) const;

        Matrix4x4 operator*(const Matrix4x4 &rhs) const;

        Matrix4x4 operator/(const Matrix4x4 &rhs) const;

        Vec3f mult(const Vec3f &rhs) const;

        Matrix4x4 mult(const Matrix4x4 &rhs) const;

        static Matrix4x4 identity();

        static Matrix4x4 translation(const Vec3f &rhs);

        static Matrix4x4 rotation(const Vec3f &axis, const Float);

        Float &operator[](uint32_t i) { return m[i / 4][i % 4]; }

        const Float &operator[](uint32_t i) const { return m[i / 4][i % 4]; }

        static bool inverse(const Matrix4x4 &in, Matrix4x4 &out);
    };

    struct Transform {
        Vec3f translation;
        Vec3f rotation;
        Float scale;

        Transform();

        Transform(const Vec3f &t, const Vec3f &r, Float s);

        //rotation then translation
        Vec3f apply(const Vec3f &) const;

        Vec3f applyRotation(const Vec3f &) const;
    };

    class CoordinateSystem {
        Vec3f localX, localZ, normal;
    public:
        CoordinateSystem() {}

        CoordinateSystem(const Vec3f &N) : normal(N) {
            const auto &w = normal;
            localX = Vec3f::cross((abs(w.x()) > 0.1) ? Vec3f{0, 1, 0} : Vec3f{1, 0, 0}, w);
            localX.normalize();
            localZ = Vec3f::cross(w, localX);
            localZ.normalize();
        }

        Vec3f worldToLocal(const Vec3f &v) const {
            return Vec3f(Vec3f::dot(localX, v), Vec3f::dot(normal, v), Vec3f::dot(localZ, v)).normalized();
        }

        Vec3f localToWorld(const Vec3f &v) const {
            return (v.x() * localX + v.y() * normal + v.z() * localZ).normalized();
        }
    };
}
#endif //MIYUKI_TRANSFORM_H
