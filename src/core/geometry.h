//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_GEOMETRY_H
#define MIYUKI_GEOMETRY_H

#include "miyuki.h"

namespace Miyuki {

    struct Float4 {
        union {
            Float v[4];
            __m128 m;
        };

        explicit Float4(Float x) { for (int32_t i = 0; i < 4; i++)v[i] = x; }

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

        Float &operator[](int32_t i) { return v[i]; }

        const Float &operator[](int32_t i) const { return v[i]; }

        static constexpr int32_t width() { return 4; }
    };

    struct Vec3f {
        Float4 v;

        Float &x() { return v[0]; }

        Float &y() { return v[1]; }

        Float &z() { return v[2]; }

        Float &w() { return v[3]; }

        Float &r() { return v[0]; }

        Float &g() { return v[1]; }

        Float &b() { return v[2]; }

        const Float &x() const { return v[0]; }

        const Float &y() const { return v[1]; }

        const Float &z() const { return v[2]; }

        const Float &r() const { return v[0]; }

        const Float &g() const { return v[1]; }

        const Float &b() const { return v[2]; }

        const Float &w() const { return v[3]; }

        void setX(Float x) {
            v[0] = x;
        }

        void setY(Float y) {
            v[1] = y;
        }

        void setZ(Float z) {
            v[2] = z;
        }

        int32_t dimension() const { return 3; }

        Float &axis(uint32_t n) {
            assert(n < 4);
            return v[n];
        }

        Float &operator[](uint32_t i) {
            return axis(i);
        }

        const Float &operator[](uint32_t i) const {
            return axis(i);
        }

        const Float &axis(uint32_t n) const {
            assert(n < 4);
            return v[n];
        }

        explicit Vec3f(Float x) { v[0] = v[1] = v[2] = x; }

        Vec3f(Float a, Float b, Float c) {
            v[0] = a;
            v[1] = b;
            v[2] = c;
            v[3] = 0;
        }

        Vec3f() { for (auto i = 0; i < 3; i++) { v[i] = 0; }}

        explicit Vec3f(const Float4 &f) : v(f) {}

        Vec3f(const Vec3f &rhs) {

            v[0] = rhs.v[0];
            v[1] = rhs.v[1];
            v[2] = rhs.v[2];
            v[3] = rhs.v[3];
        }

        explicit Vec3f(const std::initializer_list<Float> &list) {
            auto iter = list.begin();
            assert(list.size() == 3);
            for (int32_t i = 0; i < 3; i++) {
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

        static Float absDot(const Vec3f &a, const Vec3f &b) {
            return fabs(dot(a, b));
        }

        static Float matDot(const Vec3f &a, const Vec3f &b) {
            return a.x() * b.x() + a.y() * b.y() + a.z() * b.z() + a.w() * b.w();
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


    inline Vec3f rgb(int32_t r, int32_t g, int32_t b) {
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
        typedef T ValueType;
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
            for (int32_t i = 0; i < N; i++) {
                v[i] = 0;
            }
        }

        Vec(T x) {
            static_assert(N >= 1, "no x component");
            v[0] = x;
            for (int32_t i = 1; i < N; i++)
                v[i] = 0;
        }

        Vec(T x, T y) {
            static_assert(N >= 2, "no y component");
            v[0] = x;
            v[1] = y;
            for (int32_t i = 2; i < N; i++)
                v[i] = 0;
        }

        Vec(T x, T y, T z) {
            static_assert(N >= 3, "no z component");
            v[0] = x;
            v[1] = y;
            v[2] = z;
            for (int32_t i = 3; i < N; i++)
                v[i] = 0;
        }

        Vec(const Vec &rhs) {
            for (int32_t i = 0; i < N; i++)
                v[i] = rhs.v[i];

        }

        Vec(const std::initializer_list<T> &list) {
            auto iter = list.begin();
            assert(list.size() == N);
            for (int32_t i = 0; i < N; i++) {
                v[i] = *iter;
                iter++;
            }
        }

        Vec(const T _v[N]) {
            for (auto i = 0; i < N; i++)
                v[i] = _v[i];

        }

        Vec &operator+=(const Vec &rhs) {
            for (int32_t i = 0; i < N; i++)
                v[i] += rhs.v[i];
            return *this;
        }

        Vec &operator-=(const Vec &rhs) {
            for (int32_t i = 0; i < N; i++)
                v[i] -= rhs.v[i];
            return *this;
        }

        Vec &operator*=(const Vec &rhs) {
            for (int32_t i = 0; i < N; i++)
                v[i] *= rhs.v[i];
            return *this;
        }

        Vec &operator*=(const T &rhs) {
            for (int32_t i = 0; i < N; i++)
                v[i] *= rhs;
            return *this;
        }

        Vec &operator/=(const T &rhs) {
            for (int32_t i = 0; i < N; i++)
                v[i] /= rhs;
            return *this;
        }

        Vec &operator/=(const Vec &rhs) {
            for (int32_t i = 0; i < N; i++)
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

        T &operator[](int32_t i) { return v[i]; }

        const T &operator[](int32_t i) const { return v[i]; }

        size_t size() const { return N; }

        bool operator==(const Vec3f &rhs) const {
            return x() == rhs.x() && y() == rhs.y() && z() == rhs.z();
        }
    };

    using Point3f = Vec<Float, 3>;
    using Point2f = Vec<Float, 2>;
    using Point3i = Vec<int32_t, 3>;
    using Point2i = Vec<int32_t, 2>;
    const Float EPS = Float(0.00001);
    const Float INF = 1e64;

    inline Point3f operator+(const Point3f &lhs, const Vec3f &rhs) {
        return Point3f(lhs.x() + rhs.x(), lhs.y() + rhs.y(), lhs.z() + rhs.z());
    }

    inline Point3f operator-(const Point3f &lhs, const Vec3f &rhs) {
        return Point3f(lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z());
    }

    template<typename T, size_t N>
    struct Bound {
        typedef Vec<T, N> Point;
        Point pMin, pMax;

        Bound() {}

        Bound(const Point &_min, const Point &_max) : pMin(_min), pMax(_max) {}

        bool contains(const Point &point) const {
            for (int32_t i = 0; i < pMax.size(); i++) {
                if (point[i] < pMin[i] || point[i] > pMax[i])return false;
            }
            return true;
        }

        void boundingSphere(Point *center, Float *radius) const {
            *center = (pMin + pMax) / 2;
            *radius = contains(*center) ? distance(*center, pMax) : 0;
        }
    };

    using Bound3f = Bound<Float, 3>;
    using Bound2f = Bound<Float, 2>;
    using Bound3i = Bound<int, 3>;
    using Bound2i = Bound<int, 2>;

    template<typename T>
    T PointOnTriangle(const T &v1, const T &v2, const T &v3, Float u1, Float u2) {
        return v1 * (1 - u1 - u2) + v2 * u1 + v3 * u2;//v1 + u1*(v2 - v1) + u2 * (v3 - v1);
    }
    inline Vec3f fromPoint3f(const Point3f& rhs){
        return Vec3f(rhs.x(), rhs.y(), rhs.z());
    }
    inline Point3f fromVec3f(const Vec3f& rhs){
        return Point3f(rhs.x(), rhs.y(), rhs.z());
    }
}
#endif //MIYUKI_GEOMETRY_H
