#pragma once
#include "Miyuki.h"
#include <xmmintrin.h>
#include <immintrin.h>
namespace Miyuki {
	using Float = float;
	struct Float4 {
		union {
			Float v[4];
			__m128 m;
		};
		Float4(Float x) { for (int i = 0; i < 4; i++)v[i] = x; }
		Float4() {}
		Float4(__m128 x) { m = x; }
		inline Float4& operator += (const Float4 & rhs) {
			v[0] += rhs.v[0];
			v[1] += rhs.v[1];
			v[2] += rhs.v[2];
			v[3] += rhs.v[3];
			return *this;
		}
		inline Float4& operator -= (const Float4 & rhs) {
			v[0] -= rhs.v[0];
			v[1] -= rhs.v[1];
			v[2] -= rhs.v[2];
			v[3] -= rhs.v[3];
			return *this;
		}
		inline Float4& operator *= (const Float4 & rhs) {
			v[0] *= rhs.v[0];
			v[1] *= rhs.v[1];
			v[2] *= rhs.v[2];
			v[3] *= rhs.v[3];
			return *this;
		}
		inline Float4& operator /= (const Float4 & rhs) {
			v[0] /= rhs.v[0];
			v[1] /= rhs.v[1];
			v[2] /= rhs.v[2];
			v[3] /= rhs.v[3];
			return *this;
		}
		inline Float4 operator + (const Float4 & rhs)const {
			return _mm_add_ps(m, rhs.m);

		}
		Float4 operator - (const Float4 & rhs)const {
			return _mm_sub_ps(m, rhs.m);

		}
		Float4 operator * (const Float4 & rhs)const {
			return _mm_mul_ps(m, rhs.m);

		}
		Float4 operator / (const Float4 & rhs)const {
			return _mm_div_ps(m, rhs.m);
			
		}
		Float4 operator < (const Float4 & rhs) {
			return _mm_cmplt_ps(m, rhs.m);
		}
		Float4 operator <= (const Float4 & rhs) {
			return _mm_cmple_ps(m, rhs.m);
		}
		Float4 operator > (const Float4 & rhs) {
			return _mm_cmpgt_ps(m, rhs.m);
		}
		Float4 operator >= (const Float4 & rhs) {
			return _mm_cmpge_ps(m, rhs.m);
		}
		Float4 operator && (const Float4 & rhs) {
			return _mm_and_ps(m, rhs.m);
		}
		void inv() {
			v[0] = 1 / v[0];
			v[1] = 1 / v[1];
			v[2] = 1 / v[2];
			v[3] = 1 / v[3];
		}
		void abs() {
			v[0] = fabs(v[0]);
			v[1] = fabs(v[1]);
			v[2] = fabs(v[2]);
			v[3] = fabs(v[3]);
		}
		Float& operator[](int i) { return v[i]; }
		const Float& operator[](int i)const { return v[i]; }
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
		inline Float8& operator += (const Float4 & rhs) {
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
		inline Float8& operator -= (const Float4 & rhs) {
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
		inline Float8& operator *= (const Float4 & rhs) {
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
		inline Float8& operator /= (const Float8 & rhs) {
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
		inline Float8 operator + (const Float8 & rhs)const {
			return _mm256_add_ps(m, rhs.m);

		}
		Float8 operator - (const Float8 & rhs)const {
			return _mm256_sub_ps(m, rhs.m);

		}
		Float8 operator * (const Float8 & rhs)const {
			return _mm256_mul_ps(m, rhs.m);

		}
		Float8 operator / (const Float8 & rhs)const {
			return _mm256_div_ps(m, rhs.m);

		}
		Float8 operator < (const Float8 & rhs) {
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
		Float8 operator <= (const Float8 & rhs) {
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
		Float8 operator > (const Float8 & rhs) {
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
		Float8 operator >= (const Float8 & rhs) {
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
		Float8 operator && (const Float8 & rhs) {
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
		Float& operator[](int i) { return v[i]; }
		const Float& operator[](int i)const { return v[i]; }
		static constexpr int width() { return 8; }
	};
	template<typename T>
	struct pvec3 {
		using Scalar = T;
		T x, y, z;
		pvec3() {}
		pvec3(const T&_x, const T&_y, const T&_z) :x(_x),y(_y),z(_z){}
		inline pvec3 operator + (const pvec3& rhs) const{
			return pvec3(x + rhs.x, y + rhs.y, z + rhs.z);
		}
		inline pvec3 operator - (const pvec3& rhs) const {
			return pvec3(x - rhs.x, y - rhs.y, z - rhs.z);
		}
		inline static T dotProduct(const pvec3&a, const pvec3&b) {
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}
		inline static pvec3 crossProduct(const pvec3&v1, const pvec3&v2) {
			return pvec3(v1.y * v2.z - v1.z * v2.y,
				v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x);
		}
		static constexpr int width() { return T::width(); }
	};
	
	
	using vec3f4 = pvec3<Float4>;
	using vec3f8 = pvec3<Float8>;

#ifdef USE_VEC8
	using simdVec = vec3f8;
#else
	using simdVec = vec3f4;
#endif
}