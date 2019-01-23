//
// Created by Shiina Miyuki on 2019/1/12.
//

#ifndef MIYUKI_VEC_HPP
#define MIYUKI_VEC_HPP

#include <cassert>
#include <initializer_list>
#include <cmath>
#include <xmmintrin.h>
#include "util.h"

namespace Miyuki {
    struct Float4 {
        union {
            Float v[4];
            __m128 m;
        };

        explicit Float4(Float x) { for (int i = 0; i < 4; i++)v[i] = x; }

        Float4() = default;

        explicit Float4(__m128 x) { m = x; }

        inline Float4 &operator+=(const Float4 &rhs) {
            v[0] += rhs.v[0];
            v[1] += rhs.v[1];
            v[2] += rhs.v[2];
            v[3] += rhs.v[3];
            return *this;
        }

        inline Float4 &operator-=(const Float4 &rhs) {
            v[0] -= rhs.v[0];
            v[1] -= rhs.v[1];
            v[2] -= rhs.v[2];
            v[3] -= rhs.v[3];
            return *this;
        }

        inline Float4 &operator*=(const Float4 &rhs) {
            v[0] *= rhs.v[0];
            v[1] *= rhs.v[1];
            v[2] *= rhs.v[2];
            v[3] *= rhs.v[3];
            return *this;
        }

        inline Float4 &operator/=(const Float4 &rhs) {
            v[0] /= rhs.v[0];
            v[1] /= rhs.v[1];
            v[2] /= rhs.v[2];
            v[3] /= rhs.v[3];
            return *this;
        }

        inline Float4 operator+(const Float4 &rhs) const {
            return Float4(_mm_add_ps(m, rhs.m));

        }

        Float4 operator-(const Float4 &rhs) const {
            return Float4(_mm_sub_ps(m, rhs.m));

        }

        Float4 operator*(const Float4 &rhs) const {
            return Float4(_mm_mul_ps(m, rhs.m));

        }

        Float4 operator/(const Float4 &rhs) const {
            return Float4(_mm_div_ps(m, rhs.m));

        }

        Float4 operator<(const Float4 &rhs) {
            return Float4(_mm_cmplt_ps(m, rhs.m));
        }

        Float4 operator<=(const Float4 &rhs) {
            return Float4(_mm_cmple_ps(m, rhs.m));
        }

        Float4 operator>(const Float4 &rhs) {
            return Float4(_mm_cmpgt_ps(m, rhs.m));
        }

        Float4 operator>=(const Float4 &rhs) {
            return Float4(_mm_cmpge_ps(m, rhs.m));
        }

        Float4 operator&&(const Float4 &rhs) {
            return Float4(_mm_and_ps(m, rhs.m));
        }

        void inv() {
            v[0] = 1 / v[0];
            v[1] = 1 / v[1];
            v[2] = 1 / v[2];
            v[3] = 1 / v[3];
        }

        void abs() {
            v[0] = fabsf(v[0]);
            v[1] = fabsf(v[1]);
            v[2] = fabsf(v[2]);
            v[3] = fabsf(v[3]);
        }

        Float &operator[](int i) { return v[i]; }

        const Float &operator[](int i) const { return v[i]; }

        static constexpr int width() { return 4; }
    };

    struct Float8 {
        union {
            Float v[8];
            __m256 m;
        };

        Float8(Float x) { for (int i = 0; i < 8; i++)v[i] = x; }

        Float8() {}

        Float8(__m256 x) { m = x; }

        inline Float8 &operator+=(const Float8 &rhs) {
            v[0] += rhs.v[0];
            v[1] += rhs.v[1];
            v[2] += rhs.v[2];
            v[3] += rhs.v[3];
            v[4] += rhs.v[4];
            v[5] += rhs.v[5];
            v[6] += rhs.v[6];
            v[7] += rhs.v[7];
            return *this;
        }

        inline Float8 &operator-=(const Float8 &rhs) {
            v[0] -= rhs.v[0];
            v[1] -= rhs.v[1];
            v[2] -= rhs.v[2];
            v[3] -= rhs.v[3];
            v[4] -= rhs.v[4];
            v[5] -= rhs.v[5];
            v[6] -= rhs.v[6];
            v[7] -= rhs.v[7];
            return *this;
        }

        inline Float8 &operator*=(const Float8 &rhs) {
            v[0] *= rhs.v[0];
            v[1] *= rhs.v[1];
            v[2] *= rhs.v[2];
            v[3] *= rhs.v[3];
            v[4] *= rhs.v[4];
            v[5] *= rhs.v[5];
            v[6] *= rhs.v[6];
            v[7] *= rhs.v[7];
            return *this;
        }

        inline Float8 &operator/=(const Float8 &rhs) {
            v[0] /= rhs.v[0];
            v[1] /= rhs.v[1];
            v[2] /= rhs.v[2];
            v[3] /= rhs.v[3];
            v[4] /= rhs.v[4];
            v[5] /= rhs.v[5];
            v[6] /= rhs.v[6];
            v[7] /= rhs.v[7];
            return *this;
        }

        inline Float8 operator+(const Float8 &rhs) const {
            return _mm256_add_ps(m, rhs.m);

        }

        Float8 operator-(const Float8 &rhs) const {
            return _mm256_sub_ps(m, rhs.m);

        }

        Float8 operator*(const Float8 &rhs) const {
            return _mm256_mul_ps(m, rhs.m);

        }

        Float8 operator/(const Float8 &rhs) const {
            return _mm256_div_ps(m, rhs.m);

        }

        Float8 operator<(const Float8 &rhs) {
            Float8 r;
            r[0] = v[0] < rhs.v[0];
            r[1] = v[1] < rhs.v[1];
            r[2] = v[2] < rhs.v[2];
            r[3] = v[3] < rhs.v[3];
            r[4] = v[4] < rhs.v[4];
            r[5] = v[5] < rhs.v[5];
            r[6] = v[6] < rhs.v[6];
            r[7] = v[7] < rhs.v[7];
            return r;
        }

        Float8 operator<=(const Float8 &rhs) {
            Float8 r;
            r[0] = v[0] <= rhs.v[0];
            r[1] = v[1] <= rhs.v[1];
            r[2] = v[2] <= rhs.v[2];
            r[3] = v[3] <= rhs.v[3];
            r[4] = v[4] <= rhs.v[4];
            r[5] = v[5] <= rhs.v[5];
            r[6] = v[6] <= rhs.v[6];
            r[7] = v[7] <= rhs.v[7];
            return r;
        }

        Float8 operator>(const Float8 &rhs) {
            Float8 r;
            r[0] = v[0] > rhs.v[0];
            r[1] = v[1] > rhs.v[1];
            r[2] = v[2] > rhs.v[2];
            r[3] = v[3] > rhs.v[3];
            r[4] = v[4] > rhs.v[4];
            r[5] = v[5] > rhs.v[5];
            r[6] = v[6] > rhs.v[6];
            r[7] = v[7] > rhs.v[7];
            return r;
        }

        Float8 operator>=(const Float8 &rhs) {
            Float8 r;
            r[0] = v[0] >= rhs.v[0];
            r[1] = v[1] >= rhs.v[1];
            r[2] = v[2] >= rhs.v[2];
            r[3] = v[3] >= rhs.v[3];
            r[4] = v[4] >= rhs.v[4];
            r[5] = v[5] >= rhs.v[5];
            r[6] = v[6] >= rhs.v[6];
            r[7] = v[7] >= rhs.v[7];
            return r;
        }

        Float8 operator&&(const Float8 &rhs) {
            return _mm256_and_ps(m, rhs.m);
        }

        void inv() {
            v[0] = 1 / v[0];
            v[1] = 1 / v[1];
            v[2] = 1 / v[2];
            v[3] = 1 / v[3];
            v[4] = 1 / v[4];
            v[5] = 1 / v[5];
            v[6] = 1 / v[6];
            v[7] = 1 / v[7];
        }

        void abs() {
            v[0] = fabs(v[0]);
            v[1] = fabs(v[1]);
            v[2] = fabs(v[2]);
            v[3] = fabs(v[3]);
            v[4] = fabs(v[4]);
            v[5] = fabs(v[5]);
            v[6] = fabs(v[6]);
            v[7] = fabs(v[7]);
        }

        Float &operator[](int i) { return v[i]; }

        const Float &operator[](int i) const { return v[i]; }

        static constexpr int width() { return 8; }
    };

    template<typename T>
    struct pvec3 {
        using Scalar = T;
        T x, y, z;

        pvec3() {}

        pvec3(const T &_x, const T &_y, const T &_z) : x(_x), y(_y), z(_z) {}

        inline pvec3 operator+(const pvec3 &rhs) const {
            return pvec3(x + rhs.x, y + rhs.y, z + rhs.z);
        }

        inline pvec3 operator-(const pvec3 &rhs) const {
            return pvec3(x - rhs.x, y - rhs.y, z - rhs.z);
        }

        inline static T dot(const pvec3 &a, const pvec3 &b) {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        inline static pvec3 cross(const pvec3 &v1, const pvec3 &v2) {
            return pvec3(v1.y * v2.z - v1.z * v2.y,
                         v1.z * v2.x - v1.x * v2.z,
                         v1.x * v2.y - v1.y * v2.x);
        }

        static constexpr int width() { return T::width(); }
    };


    using vec3f4 = pvec3<Float4>;
    using vec3f8 = pvec3<Float8>;
//#define USE_VEC8
#ifdef USE_VEC8
    using simdVec = vec3f8;
#else
    using simdVec = vec3f4;
#endif

    struct Vec3f {
        Float4 v;

        Float &x() { return v[0]; }

        Float &y() { return v[1]; }

        Float &z() { return v[2]; }

        Float &r() { return v[0]; }

        Float &g() { return v[1]; }

        Float &b() { return v[2]; }

        const Float &x() const { return v[0]; }

        const Float &y() const { return v[1]; }

        const Float &z() const { return v[2]; }

        const Float &r() const { return v[0]; }

        const Float &g() const { return v[1]; }

        const Float &b() const { return v[2]; }

        void setX(Float x) {
            v[0] = x;
        }

        void setY(Float y) {
            v[1] = y;
        }

        void setZ(Float z) {
            v[2] = z;
        }

        int dimension() const { return 3; }

        Float &axis(unsigned int n) {
            assert(n < 3);
            return v[n];
        }

        Float &operator[](unsigned int i) {
            return axis(i);
        }

        const Float &operator[](unsigned int i) const {
            return axis(i);
        }

        const Float &axis(unsigned int n) const {
            assert(n < 3);
            return v[n];
        }

        Vec3f(Float a, Float b, Float c) {
            v[0] = a;
            v[1] = b;
            v[2] = c;
            v[3] = 0;
        }

        Vec3f() { for (auto i = 0; i < 3; i++) { v[i] = 0; }}

        Vec3f(const Float4 &f) : v(f) {}

        Vec3f(const Vec3f &rhs) {

            v[0] = rhs.v[0];
            v[1] = rhs.v[1];
            v[2] = rhs.v[2];
            v[3] = rhs.v[3];
        }

        Vec3f(const std::initializer_list<Float> &list) {
            auto iter = list.begin();
            assert(list.size() == 3);
            for (int i = 0; i < 3; i++) {
                v[i] = *iter;
                iter++;
            }
            v[3] = 0;
        }

        Vec3f(const Float _v[3]) {
            for (auto i = 0; i < 3; i++)
                v[i] = _v[i];
            v[3] = 0;
        }

        Vec3f &operator+=(const Vec3f &rhs) {
            v += rhs.v;
            return *this;
        }

        Vec3f &operator-=(const Vec3f &rhs) {
            v -= rhs.v;
            return *this;
        }

        Vec3f &operator*=(const Vec3f &rhs) {
            v *= rhs.v;
            return *this;
        }

        Vec3f &operator*=(const Float &rhs) {
            v[0] *= rhs;
            v[1] *= rhs;
            v[2] *= rhs;
            v[3] *= rhs;
            return *this;
        }

        Vec3f &operator/=(const Float &rhs) {
            v[0] /= rhs;
            v[1] /= rhs;
            v[2] /= rhs;
            v[3] /= rhs;
            return *this;
        }

        Vec3f &operator/=(const Vec3f &rhs) {
            v /= rhs.v;
            return *this;
        }

        Vec3f operator*(const Vec3f &rhs) const {
            return Vec3f(v * rhs.v);

        }

        Vec3f operator/(const Vec3f &rhs) const {
            return Vec3f(v / rhs.v);

        }

        Vec3f operator+(const Vec3f &rhs) const {
            return Vec3f(v + rhs.v);

        }

        Vec3f operator-(const Vec3f &rhs) const {
            return Vec3f(v - rhs.v);

        }

        Float max() const {
            return std::max(x(), std::max(y(), z()));
        }

        Float min() const {
            return std::min(x(), std::min(y(), z()));
        }

        static Float dot(const Vec3f &a, const Vec3f &b) {
            return a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
        }

        static Vec3f cross(const Vec3f &v1, const Vec3f &v2) {
            return Vec3f{v1.y() * v2.z() - v1.z() * v2.y(),
                         v1.z() * v2.x() - v1.x() * v2.z(),
                         v1.x() * v2.y() - v1.y() * v2.x()};
        }

        Float lengthSquared() const {
            return Vec3f::dot(*this, *this);
        }

        Float length() const {
            return sqrt(lengthSquared());
        }

        void normalize() {
            *this /= length();
        }

        const Vec3f normalized() const {
            auto v = *this;
            v.normalize();
            return v;
        }
    };

    inline Float luminance(const Vec3f &a) {
        return Float(0.299) * a.x() + Float(0.587) * a.y() + Float(0.114) * a.z();
    }

    inline Vec3f operator*(const Vec3f &a, Float k) {
        Vec3f _v(a);
        _v *= k;
        return _v;
    }

    inline Vec3f operator*(Float k, const Vec3f &a) {
        return a * k;
    }

    inline Vec3f operator/(const Vec3f &a, Float k) {
        Vec3f _v(a);
        _v /= k;
        return _v;
    }


    inline Float clamp(Float x, Float a, Float b) {
        if (x < a)return a;
        if (x > b)return b;
        return x;
    }

    inline Vec3f min(const Vec3f &a, const Vec3f &b) {
        return Vec3f(std::min(a.x(), b.x()),
                     std::min(a.y(), b.y()),
                     std::min(a.z(), b.z()));
    }

    inline Vec3f max(const Vec3f &a, const Vec3f &b) {
        return Vec3f(std::max(a.x(), b.x()),
                     std::max(a.y(), b.y()),
                     std::max(a.z(), b.z()));
    }


    inline Vec3f rgb(int r, int g, int b) {
        return Vec3f{(Float) r, (Float) g, (Float) b} / 255.0;
    }

    /// axis is normalized
    inline Vec3f rotate(const Vec3f &x, const Vec3f &axis, const Float angle) {
        const Float s = sin(angle);
        const Float c = cos(angle);
        const Float oc = Float(1.0) - c;
        Vec3f u({oc * axis.x() * axis.x() + c, oc * axis.x() * axis.y() - axis.z() * s,
                 oc * axis.z() * axis.x() + axis.y() * s});
        Vec3f v({oc * axis.x() * axis.y() + axis.z() * s, oc * axis.y() * axis.y() + c,
                 oc * axis.y() * axis.z() - axis.x() * s});
        Vec3f w({oc * axis.z() * axis.x() - axis.y() * s, oc * axis.y() * axis.z() + axis.x() * s,
                 oc * axis.z() * axis.z() + c});
        return Vec3f{Vec3f::dot(u, x), Vec3f::dot(v, x), Vec3f::dot(w, x)};
    }


    template<typename T, size_t N>
    struct Vec {
        T v[N];

        T &x() { static_assert(N >= 1, "no x component"); return v[0]; }

        T &y() { static_assert(N >= 2, "no y component");return v[1]; }

        T &z() { static_assert(N >= 3, "no z component");return v[2]; }

        T &r() { static_assert(N >= 1, "no r component");return v[0]; }

        T &g() { static_assert(N >= 2, "no g component");return v[1]; }

        T &b() { static_assert(N >= 3, "no b component"); return v[2]; }

        const T &x() const { static_assert(N >= 1, "no x component"); return v[0]; }

        const T &y() const { static_assert(N >= 2, "no y component");return v[1]; }

        const T &z() const { static_assert(N >= 3, "no z component");return v[2]; }

        const T &r() const { static_assert(N >= 1, "no r component");return v[0]; }

        const T &g() const { static_assert(N >= 2, "no g component");return v[1]; }

        const T &b() const { static_assert(N >= 3, "no b component");return v[2]; }

        Vec() {
            for (int i = 0; i < N; i++) {
                v[i] = 0;
            }
        }

        Vec(T x) {
            static_assert(N >= 1, "no x component");
            v[0] = x;
            for (int i = 1; i < N; i++)
                v[i] = 0;
        }

        Vec(T x, T y) {
            static_assert(N >= 2, "no y component");
            v[0] = x;
            v[1] = y;
            for (int i = 2; i < N; i++)
                v[i] = 0;
        }

        Vec(T x, T y, T z) {
            static_assert(N >= 3, "no z component");
            v[0] = x;
            v[1] = y;
            v[2] = z;
            for (int i = 3; i < N; i++)
                v[i] = 0;
        }

        Vec(const Vec &rhs) {
            for (int i = 0; i < N; i++)
                v[i] = rhs.v[i];

        }

        Vec(const std::initializer_list<T> &list) {
            auto iter = list.begin();
            assert(list.size() == N);
            for (int i = 0; i < N; i++) {
                v[i] = *iter;
                iter++;
            }
        }

        Vec(const T _v[N]) {
            for (auto i = 0; i < N; i++)
                v[i] = _v[i];

        }

        Vec &operator+=(const Vec &rhs) {
            for (int i = 0; i < N; i++)
                v[i] += rhs.v[i];
            return *this;
        }

        Vec &operator-=(const Vec &rhs) {
            for (int i = 0; i < N; i++)
                v[i] -= rhs.v[i];
            return *this;
        }

        Vec &operator*=(const Vec &rhs) {
            for (int i = 0; i < N; i++)
                v[i] *= rhs.v[i];
            return *this;
        }

        Vec &operator*=(const T &rhs) {
            for (int i = 0; i < N; i++)
                v[i] *= rhs;
            return *this;
        }

        Vec &operator/=(const T &rhs) {
            for (int i = 0; i < N; i++)
                v[i] /= rhs;
            return *this;
        }

        Vec &operator/=(const Vec &rhs) {
            for (int i = 0; i < N; i++)
                v[i] /= rhs.v[i];
            return *this;
        }

        Vec operator*(const Vec &rhs) const {
            auto x = *this;
            x *= rhs;
            return x;
        }

        Vec operator/(const Vec &rhs) const {
            auto x = *this;
            x /= rhs;
            return x;
        }

        Vec operator*(const T &rhs) const {
            auto x = *this;
            x *= rhs;
            return x;
        }

        Vec operator/(const T &rhs) const {
            auto x = *this;
            x /= rhs;
            return x;
        }

        Vec operator+(const Vec &rhs) const {
            auto x = *this;
            x += rhs;
            return x;
        }

        Vec operator-(const Vec &rhs) const {
            auto x = *this;
            x -= rhs;
            return x;
        }

        T &operator[](int i) { return v[i]; }

        const T &operator[](int i) const { return v[i]; }

        int size() const { return N; }
    };

    using Point3f = Vec<Float, 3>;
    using Point2f = Vec<Float, 2>;
    using Point3i = Vec<int, 3>;
    using Point2i = Vec<int, 2>;
    const Float EPS = Float(0.001);
    const Float INF = 1e64;

    template<typename T>
    struct Bound {
        T pMin, pMax;

        Bound(const T &_min, const T &_max) : pMin(_min), pMax(_max) {}

        bool contains(const T &point) {
            for (int i = 0; i < pMax.size(); i++) {
                if (point[i] < pMin[i] || point[i] > pMax[i])return false;
            }
            return true;
        }
    };

    using Bound3f = Bound<Point3f>;
    using Bound2f = Bound<Point2f>;
    using Bound3i = Bound<Point3i>;
    using Bound2i = Bound<Point2i>;

    Vec3f cosineWeightedHemisphereSampling(const Vec3f &norm, Float u1, Float u2);

    Vec3f sphereSampling(Float u1, Float u2);

    Vec3f GGXImportanceSampling(Float roughness, const Vec3f &norm, Float u1, Float u2);

    Float GGXDistribution(const Vec3f &m, const Vec3f &n, float alpha_g);

    Vec3f pointOnTriangle(const Vec3f &v1, const Vec3f &v2, const Vec3f &v3, Float u1, Float u2);

    const Float PI = 3.1415926535f;
    const Float INVPI = 1.0 / PI;
    const Float PI2 = PI / 2;
}
#endif //MIYUKI_VEC_HPP
