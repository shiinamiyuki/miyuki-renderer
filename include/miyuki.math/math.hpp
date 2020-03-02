// MIT License
//
// Copyright (c) 2019 椎名深雪
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MIYUKI_MATH_MATH_HPP
#define MIYUKI_MATH_MATH_HPP

#include <array>
#include <type_traits>
#include <functional>
#include <xmmintrin.h>
#include <immintrin.h>
#include <cstring>
#include <cmath>

namespace miyuki::math {
    using std::sin;
    using std::cos;
    using std::tan;
    using std::atan;
    using std::exp;
    using std::log;
    using std::asin;
    using std::acos;
    using std::sqrt;
    using std::abs;
    using std::pow;
    using std::max;
    using std::min;
    using std::floor;
    using std::ceil;


    template<class T, int N>
    class Array;

    namespace detail {
        template<class T, int N>
        struct to_array {
            using type = Array<T, N>;

        };
    }


    template<class F, int N, class... Args>
    auto
    array_forward(F &&f,
          const Array<Args, N> &... args) -> typename detail::to_array<std::invoke_result_t<F, Args...>, N>::type {
        typename detail::to_array<std::invoke_result_t<F, Args...>, N>::type ret;
        for (auto i = 0; i < N; i++) {
            ret[i] = f(args[i]...);
        }
        return ret;
    }

#define MYK_VEC_GEN_MATH_FUNC(func)  friend self_type func (const self_type & v){\
                                        return array_forward([](const value_type &a)->value_type{\
                                                    return func(a);\
                                                }, v); \
                                        }
#define MYK_VEC_GEN_MATH_FUNC2(func) friend self_type func(const self_type & lhs,const self_type & rhs ) {\
                                        return array_forward([](const value_type &a, const value_type &b)->value_type{\
                                            return func(a,b);\
                                        }, lhs, rhs); \
                                    }
#define MYK_VEC_GEN_MATH_FUNCS() \
    MYK_VEC_GEN_MATH_FUNC(sin) \
    MYK_VEC_GEN_MATH_FUNC(cos) \
    MYK_VEC_GEN_MATH_FUNC(tan) \
    MYK_VEC_GEN_MATH_FUNC(atan) \
    MYK_VEC_GEN_MATH_FUNC(exp) \
    MYK_VEC_GEN_MATH_FUNC(log) \
    MYK_VEC_GEN_MATH_FUNC(asin) \
    MYK_VEC_GEN_MATH_FUNC(acos) \
    MYK_VEC_GEN_MATH_FUNC(sqrt) \
    MYK_VEC_GEN_MATH_FUNC(abs) \
    MYK_VEC_GEN_MATH_FUNC(floor) \
    MYK_VEC_GEN_MATH_FUNC(ceil) \
    MYK_VEC_GEN_MATH_FUNC2(pow)


#define MYK_VEC_ARR_GEN_OP_(op) friend self_type operator op (const self_type & lhs,const self_type & rhs ) {\
                                        return array_forward([](const value_type &a, const value_type &b)->value_type{\
                                            return value_type(a op b);\
                                        }, lhs, rhs); \
                                }
#define MYK_VEC_ARR_GEN_OP(op) MYK_VEC_ARR_GEN_OP_(op)
#define MYK_VEC_ARR_GEN_ASSIGN_OP(op) self_type& operator op##= (const self_type & rhs){*this = *this op rhs;return *this;}

#define MYK_VEC_GEN_BASIC_ASSIGN_OPS()  \
    MYK_VEC_ARR_GEN_ASSIGN_OP(+) \
    MYK_VEC_ARR_GEN_ASSIGN_OP(-)\
    MYK_VEC_ARR_GEN_ASSIGN_OP(*)\
    MYK_VEC_ARR_GEN_ASSIGN_OP(/)

#define MYK_VEC_GEN_BASIC_OPS() \
    MYK_VEC_ARR_GEN_OP_(+) \
    MYK_VEC_ARR_GEN_OP_(-) \
    MYK_VEC_ARR_GEN_OP_(*) \
    MYK_VEC_ARR_GEN_OP_(/) \
    MYK_VEC_ARR_GEN_OP_(==)\
    MYK_VEC_ARR_GEN_OP_(!=)\
    MYK_VEC_ARR_GEN_OP_(<) \
    MYK_VEC_ARR_GEN_OP_(<=)\
    MYK_VEC_ARR_GEN_OP_(>) \
    MYK_VEC_ARR_GEN_OP_(>=)

#define MYK_VEC_GEN_ACCESS(Name, Index) \
    value_type Name ()const {static_assert(Index < N, "component "#Name " is not available");return (*this)[Index];} \
    value_type& Name() {static_assert(Index < N, "component "#Name " is not available");return (*this)[Index];}

#define MYK_VEC_GEN_ACCESS_FUNCS() \
    MYK_VEC_GEN_ACCESS(x, 0) \
    MYK_VEC_GEN_ACCESS(y, 1) \
    MYK_VEC_GEN_ACCESS(z, 2) \
    MYK_VEC_GEN_ACCESS(w, 3) \
    MYK_VEC_GEN_ACCESS(r, 0) \
    MYK_VEC_GEN_ACCESS(g, 1) \
    MYK_VEC_GEN_ACCESS(b, 2) \
    MYK_VEC_GEN_ACCESS(a, 3)

    template<class T, int N>
    class Array : public std::array<T, N> {
        using value_type = T;
        using self_type = Array<T, N>;
    public:
        Array() = default;


        template<class U, int M, typename = std::enable_if_t<std::is_convertible_v<U, T> && M <= N, void>>
        Array(const Array<U, M> &v) {
            for (int i = 0; i < M; i++) {
                (*this)[i] = (T) v[i];
            }
        }

        Array(std::initializer_list<T> list) {
            auto it = list.begin();
            for (int i = 0; i < 4; i++) {
                (*this)[i] = *it;
                it++;
            }
        }

        template<class First, class Second, class... Rest>
        Array(const First &first, const Second &second, const Rest &... rest):std::array<T, N>(
                {T(first), T(second), T(rest)...}) {}

        Array(const T &value) {
            for (auto &i: *this) {
                i = value;
            }
        }

        MYK_VEC_GEN_BASIC_OPS()

        MYK_VEC_GEN_BASIC_ASSIGN_OPS()

        MYK_VEC_GEN_ACCESS_FUNCS()

    };


    template<int N>
    class Array<float, N> : public std::array<float, N> {
        using value_type = float;
        using self_type = Array;
    public:
        Array() = default;

        template<class U, int M, typename = std::enable_if_t<std::is_convertible_v<U, float> && M <= N, void>>
        Array(const Array<U, M> &v) {
            for (int i = 0; i < M; i++) {
                (*this)[i] = (float) v[i];
            }
        }

        template<class T>
        Array(std::initializer_list<T> list) {
            auto it = list.begin();
            for (int i = 0; i < 4; i++) {
                (*this)[i] = *it;
                it++;
            }
        }

        template<class First, class Second, class... Rest>
        Array(const First &first, const Second &second, const Rest &... rest):std::array<float, N>(
                {float(first), float(second), float(rest)...}) {}

        Array(const float &value) {
            for (auto &i: *this) {
                i = value;
            }
        }

        MYK_VEC_GEN_BASIC_OPS()

        MYK_VEC_GEN_BASIC_ASSIGN_OPS()

        MYK_VEC_GEN_MATH_FUNCS()

        MYK_VEC_GEN_ACCESS_FUNCS()
    };


    template<class Derived>
    class Float4Base {
        using self_type = Float4Base;
        using value_type = float;
    public:
        union {
            float s[4];
            __m128 m;
        };

        float &operator[](int i) {
            return s[i];
        }

        const float &operator[](int i) const {
            return s[i];
        }

        Float4Base() = default;

        Float4Base(const float &v) : m(_mm_broadcast_ss(&v)) {}

        Float4Base(std::initializer_list<float> list) {
            auto it = list.begin();
            for (int i = 0; i < 4; i++) {
                (*this)[i] = *it;
                it++;
            }
        }

        Float4Base(__m128 m) {
            auto n = m;
            memcpy(this, &n, sizeof(__m128));
        }

        operator __m128() const {
            __m128 m;
            memcpy(&m, this, sizeof(__m128));
            return m;
        };

        friend Derived operator+(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm_add_ps((__m128) lhs, (__m128) rhs));
        }

        friend Derived operator-(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm_sub_ps((__m128) lhs, (__m128) rhs));
        }

        friend Derived operator*(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm_mul_ps((__m128) lhs, (__m128) rhs));
        }

        friend Derived operator/(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm_div_ps((__m128) lhs, (__m128) rhs));
        }

        friend Derived operator<(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm_cmplt_ps((__m128) lhs, (__m128) rhs));
        }

        friend Derived operator<=(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm_cmple_ps((__m128) lhs, (__m128) rhs));
        }

        friend Derived operator>(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm_cmpgt_ps((__m128) lhs, (__m128) rhs));
        }

        friend Derived operator>=(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm_cmpge_ps((__m128) lhs, (__m128) rhs));
        }


        MYK_VEC_GEN_BASIC_ASSIGN_OPS()


    };


    template<class Derived>
    class Float8Base {
        using self_type = Float8Base<Derived>;
        using value_type = float;
    public:
        union {
            float s[8];
            __m256 m;
            struct {
                __m128 lo, hi;
            };
        };

        Float8Base(const float &v) : m(_mm256_broadcast_ss(&v)) {}

        Float8Base(__m256 m) {
            auto n = m;
            memcpy(&this->m, &n, sizeof(__m256));
        }

        operator __m256() const {
            __m256 m;
            memcpy(&m, &this->m, sizeof(__m256));
            return m;
        };

        friend Derived operator+(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm256_add_ps((__m256) lhs, (__m256) rhs));
        }

        friend Derived operator-(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm256_sub_ps((__m256) lhs, (__m256) rhs));
        }

        friend Derived operator*(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm256_mul_ps((__m256) lhs, (__m256) rhs));
        }

        friend Derived operator/(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm256_div_ps((__m256) lhs, (__m256) rhs));
        }

        friend Derived operator<(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm256_cmp_ps((__m256) lhs, (__m256) rhs, _CMP_LT_OQ));
        }

        friend Derived operator<=(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm256_cmp_ps((__m256) lhs, (__m256) rhs, _CMP_LE_OQ));
        }

        friend Derived operator>(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm256_cmp_ps((__m256) lhs, (__m256) rhs, _CMP_GT_OQ));
        }

        friend Derived operator>=(const Derived &lhs, const Derived &rhs) {
            return Derived(_mm256_cmp_ps((__m256) lhs, (__m256) rhs, _CMP_GT_OQ));
        }

        MYK_VEC_GEN_BASIC_ASSIGN_OPS()
    };

    // static_assert(sizeof(__m128) == sizeof(Float4Base));


    template<>
    class Array<float, 3> : public Float4Base<Array<float, 3>> {
        static const int N = 3;
        using self_type = Array<float, 3>;
        using value_type = float;
    public:
        Array(const Float4Base &v) : Float4Base(v) {}

        Array(const float x, float y, float z) {
            this->s[0] = x;
            this->s[1] = y;
            this->s[2] = z;
        }

        using Float4Base::Float4Base;

        MYK_VEC_GEN_ACCESS(x, 0)

        MYK_VEC_GEN_ACCESS(y, 1)

        MYK_VEC_GEN_ACCESS(z, 2)

        MYK_VEC_GEN_ACCESS(r, 0)

        MYK_VEC_GEN_ACCESS(g, 1)

        MYK_VEC_GEN_ACCESS(b, 2)

        MYK_VEC_GEN_MATH_FUNCS()
    };

    static_assert(sizeof(Array<float, 3>) == sizeof(__m128));

    template<>
    class Array<float, 4> : public Float4Base<Array<float, 4>> {
        static const int N = 4;
        using self_type = Array<float, 4>;
        using value_type = float;
    public:
        Array(const Array<float, 3> &v, float w) {
            memcpy(&m, &v, sizeof(float) * 3);
            (*this)[3] = w;
        }

        Array(const Float4Base &v) : Float4Base(v) {}

        using Float4Base::Float4Base;

        MYK_VEC_GEN_ACCESS_FUNCS()

        MYK_VEC_GEN_MATH_FUNCS()
    };

    static_assert(sizeof(Array<float, 4>) == sizeof(__m128));

    template<class T>
    struct array_length {
        static const int value = 1;
    };

    template<class T, int N>
    struct array_length<Array<T, N>> {
        static const int value = N;
    };

    template<class T, int N>
    struct to_array {
        using type = Array<T, N>;
    };

    template<class T>
    struct to_array<T, 1> {
        using type = T;
    };

    template<class T, int N>
    using TArray = typename to_array<T, N>::type;


    template<class T>
    inline constexpr int LengthOf = array_length<T>::value;

    template<class T, int N>
    T dot(const Array<T, N> &a, const Array<T, N> &b) {
        auto x = a[0] * b[0];
        for (auto i = 1; i < N; i++) {
            x += a[i] * b[i];
        }
        return x;
    }

    template<class T, int N>
    T length(const Array<T, N> &v) {
        return sqrt(dot(v, v));
    }

    template<class T, int N>
    Array<T, N> normalize(const Array<T, N> &v) {
        return v / length(v);
    }

    template<class T>
    Array<T, 3> cross(const Array<T, 3> &v1, const Array<T, 3> &v2) {
        return {v1[1] * v2[2] - v1[2] * v2[1], v1[2] * v2[0] - v1[0] * v2[2], v1[0] * v2[1] - v1[1] * v2[0]};
    }

    template<class T, int N>
    Array<T, N> clamp(const Array<T, N> &x, const Array<T, N> &a, const Array<T, N> &b) {
        return array_forward([](const T &a, const T &b, const T &c) -> T { return std::clamp(a, b, c); }, x, a, b);
    }

    template<class T, int N>
    Array<T, N> min(const Array<T, N> &a, const Array<T, N> &b) {
        return array_forward([](const T &a, const T &b) -> T { return std::min(a, b); }, a, b);
    }


    template<class T, int N>
    Array<T, N> max(const Array<T, N> &a, const Array<T, N> &b) {
        return array_forward([](const T &a, const T &b) -> T { return std::max(a, b); }, a, b);
    }

    template<class T, int N>
    Array<T, N> mod(const Array<T, N> &x, const Array<T, N> &y) {
        return x - y * floor(x / y);
    }

    template<class T>
    class Matrix4 {
        Array<Array<T, 4>, 4> _rows;
        static_assert(sizeof(_rows) == sizeof(T) * 16);
        using Vec3 = Array<T, 3>;
    public:
        static Matrix4 zero() {
            Matrix4 matrix4;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    matrix4[i][j] = T(0.0f);
                }
            }
            return matrix4;
        }

        static Matrix4 identity() {
            Matrix4 matrix4;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    matrix4[i][j] = i == j ? T(1.0f) : T(0.0f);
                }
            }
            return matrix4;
        }

        static Matrix4 scale(const T &k) {
            Matrix4 matrix4;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    matrix4[i][j] = i == j ? k : T(0.0f);
                }
            }
            return matrix4;
        }

        static Matrix4 rotate(const T &angle, const Vec3 &axis) {
            const auto zero = T(0.0f);
            const auto one = T(1.0f);
            const T s = sin(angle);
            const T c = cos(angle);
            const T oc = one - c;
            T r[4][4] = {
                    {oc * axis.x() * axis.x() + c,            oc * axis.x() * axis.y() - axis.z() * s,
                                                                                                       oc * axis.z() *
                                                                                                       axis.x() +
                                                                                                       axis.y() *
                                                                                                       s,    zero},
                    {oc * axis.x() * axis.y() + axis.z() * s, oc * axis.y() * axis.y() + c,            oc * axis.y() *
                                                                                                       axis.z() -
                                                                                                       axis.x() *
                                                                                                       s,    zero},
                    {oc * axis.z() * axis.x() - axis.y() * s, oc * axis.y() * axis.z() + axis.x() * s, oc * axis.z() *
                                                                                                       axis.z() +
                                                                                                       c,    zero},
                    {zero,                                    zero,                                    zero, one}};
            Matrix4 m;
            std::memcpy(&m, &r, sizeof(_rows));
            return m;
        }

        static Matrix4 translate(const Vec3 &v) {
            Matrix4 m = identity();
            m[0][3] = v[0];
            m[1][3] = v[1];
            m[2][3] = v[2];
            return m;
        }

        Matrix4 inverse() const {
            auto m = reinterpret_cast<const T *>(&_rows);
            T inv[16], det;
            int i;


            inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +
                     m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

            inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -
                     m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

            inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +
                     m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

            inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -
                      m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

            inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] -
                     m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

            inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] +
                     m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

            inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] -
                     m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

            inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] +
                      m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

            inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
                     m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

            inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] -
                     m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

            inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] +
                      m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

            inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] -
                      m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

            inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -
                     m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

            inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
                     m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

            inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
                      m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

            inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
                      m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

            det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

            det = T(1.0) / det;

            Matrix4 <T> out;
            auto invOut = reinterpret_cast<T *>(&out._rows);
            for (i = 0; i < 16; i++)
                invOut[i] = inv[i] * det;
            return out;
        }

        Matrix4 operator*(const Matrix4 &rhs) const {
            Matrix4 m;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    m._rows[i][j] = dot(_rows[i], rhs.column(j));
                }
            }
            return m;
        }

        Matrix4 &operator*=(const Matrix4 &rhs) {
            auto m = *this * rhs;
            std::memcpy(this, m, sizeof(T) * 16);
            return *this;
        }

        Array<T, 4> operator*(const Array<T, 4> &v) const {
            return Array<T, 4>{dot(_rows[0], v),
                               dot(_rows[1], v),
                               dot(_rows[2], v),
                               dot(_rows[3], v)};
        }

        Array<T, 4> column(int i) const {
            return Array<T, 4>{_rows[0][i], _rows[1][i], _rows[2][i], _rows[3][i]};
        }

        const Array<T, 4> &operator[](int i) const {
            return _rows[i];
        }

        Array<T, 4> &operator[](int i) {
            return _rows[i];
        }

        Matrix4 transpose() const {
            Matrix4 m;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    m[i][j] = (*this)[j][i];
                }
            }
            return m;
        }
    };

    template<class T>
    class Matrix3 {
        Array<Array<T, 3>, 3> _rows;
        using Vec3 = Array<T, 3>;
    public:
        Matrix3() = default;

        explicit Matrix3(const Matrix4<T> &mat4) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    _rows[i][j] = mat4[i][j];
                }
            }
        }

        Matrix3 transpose() const {
            Matrix3 m;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    m[i][j] = (*this)[j][i];
                }
            }
            return m;
        }

        static Matrix3 zero() {
            Matrix3 m;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    m[i][j] = T(0.0f);
                }
            }
            return m;
        }

        static Matrix3 identity() {
            Matrix3 m;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    m[i][j] = i == j ? T(1.0f) : T(0.0f);
                }
            }
            return m;
        }

        Matrix3 inverse() const {
            T det = (*this)[0][0] * (*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2] -
                    (*this)[0][1] * (*this)[1][0] *
                    (*this)[2][2] - (*this)[1][2] * (*this)[2][0] +
                    (*this)[0][2] * (*this)[1][0] * (*this)[2][1] - (*this)[1][1] * (*this)[2][0];

            T invdet = T(1.0f) / det;

            Matrix3 inv; // inverse of matrix m
            inv[0][0] = (*this)[1][1] * (*this)[2][2] - (*this)[2][1] * (*this)[1][2] * invdet;
            inv[0][1] = (*this)[0][2] * (*this)[2][1] - (*this)[0][1] * (*this)[2][2] * invdet;
            inv[0][2] = (*this)[0][1] * (*this)[1][2] - (*this)[0][2] * (*this)[1][1] * invdet;
            inv[1][0] = (*this)[1][2] * (*this)[2][0] - (*this)[1][0] * (*this)[2][2] * invdet;
            inv[1][1] = (*this)[0][0] * (*this)[2][2] - (*this)[0][2] * (*this)[2][0] * invdet;
            inv[1][2] = (*this)[1][0] * (*this)[0][2] - (*this)[0][0] * (*this)[1][2] * invdet;
            inv[2][0] = (*this)[1][0] * (*this)[2][1] - (*this)[2][0] * (*this)[1][1] * invdet;
            inv[2][1] = (*this)[2][0] * (*this)[0][1] - (*this)[0][0] * (*this)[2][1] * invdet;
            inv[2][2] = (*this)[0][0] * (*this)[1][1] - (*this)[1][0] * (*this)[0][1] * invdet;
            return inv;
        }

        const Array<T, 3> &operator[](size_t i) const {
            return _rows[i];
        }

        Array<T, 3> &operator[](size_t i) {
            return _rows[i];
        }

        Matrix3 operator*(const Matrix3 &rhs) const {
            Matrix3 m;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    m._rows[i][j] = dot(_rows[i], rhs.column(j));
                }
            }
            return m;
        }

        Matrix3 &operator*=(const Matrix3 &rhs) {
            auto m = *this * rhs;
            std::memcpy(this, m, sizeof(T) * 9);
            return *this;
        }

        Array<T, 3> operator*(const Array<T, 3> &v) const {
            return Array<T, 3>{dot(_rows[0], v),
                               dot(_rows[1], v),
                               dot(_rows[2], v)};
        }

    };

    template<class T>
    Matrix4<T> inverse(const Matrix4<T> &m) {
        return m.inverse();
    }

    template<class T>
    Matrix3<T> inverse(const Matrix3<T> &m) {
        return m.inverse();
    }

    template<class T>
    Matrix4<T> transpose(const Matrix4<T> &m) {
        return m.transpose();
    }

    template<class T>
    Matrix3<T> transpose(const Matrix3<T> &m) {
        return m.transpose();
    }
}

namespace miyuki {
#define MYK_VEC_DECL_PRIMITIVE(TY, N) using TY##N = math::Array<TY, N>;
    MYK_VEC_DECL_PRIMITIVE(bool, 2)
    MYK_VEC_DECL_PRIMITIVE(bool, 3)
    MYK_VEC_DECL_PRIMITIVE(bool, 4)
    MYK_VEC_DECL_PRIMITIVE(bool, 8)
    MYK_VEC_DECL_PRIMITIVE(bool, 16)

    MYK_VEC_DECL_PRIMITIVE(float, 2)
    MYK_VEC_DECL_PRIMITIVE(float, 3)
    MYK_VEC_DECL_PRIMITIVE(float, 4)
    MYK_VEC_DECL_PRIMITIVE(float, 8)
    MYK_VEC_DECL_PRIMITIVE(float, 16)

    MYK_VEC_DECL_PRIMITIVE(int, 2)
    MYK_VEC_DECL_PRIMITIVE(int, 3)
    MYK_VEC_DECL_PRIMITIVE(int, 4)
    MYK_VEC_DECL_PRIMITIVE(int, 8)
    MYK_VEC_DECL_PRIMITIVE(int, 16)

}


#endif //MIYUKI_MATH_MATH_HPP
