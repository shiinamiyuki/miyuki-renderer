//
// Created by xiaoc on 2018/10/31.
//

#ifndef VEC_H
#define VEC_H

#include <cassert>
#include <initializer_list>
#include <cmath>
#include <xmmintrin.h>
#include "Float4.h"
namespace Miyuki {

	struct vec3 {
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
		Float operator [](unsigned int i) {
			return axis(i);
		}
		const Float &axis(unsigned int n) const {
			assert(n < 3);
			return v[n];
		}
		vec3(Float a, Float b, Float c) {
			v[0] = a;
			v[1] = b;
			v[2] = c;
			v[3] = 0;
		}
		vec3() { for (auto i = 0; i < 3; i++) { v[i] = 0; } }
		vec3(const Float4 & f) :v(f) {}
		vec3(const vec3 &rhs) {
			
			v[0] = rhs.v[0];
			v[1] = rhs.v[1];
			v[2] = rhs.v[2];
			v[3] = rhs.v[3];
		}

		vec3(const std::initializer_list<Float> &list) {
			auto iter = list.begin();
			assert(list.size() == 3);
			for (int i = 0; i < 3; i++) {
				v[i] = *iter;
				iter++;
			}
			v[3] = 0;
		}

		vec3(const Float _v[3]) {
			for (auto i = 0; i < 3; i++)
				v[i] = _v[i];
			v[3] = 0;
		}

		vec3 &operator+=(const vec3 &rhs) {
			v += rhs.v;
			return *this;
		}

		vec3 &operator-=(const vec3 &rhs) {
			v -= rhs.v;
			return *this;
		}

		vec3 &operator*=(const vec3 &rhs) {
			v *= rhs.v;
			return *this;
		}

		vec3 &operator*=(const Float &rhs) {
			v[0] *= rhs;
			v[1] *= rhs;
			v[2] *= rhs;
			v[3] *= rhs;
			return *this;
		}

		vec3 &operator/=(const Float &rhs) {
			v[0] /= rhs;
			v[1] /= rhs;
			v[2] /= rhs;
			v[3] /= rhs;
			return *this;
		}

		vec3 &operator/=(const vec3 &rhs) {
			v /= rhs.v;
			return *this;
		}

		vec3 operator*(const vec3 &rhs) const {
			return vec3(v * rhs.v);
	
		}

		vec3 operator/(const vec3 &rhs) const {
			return vec3(v / rhs.v);

		}

		vec3 operator+(const vec3 &rhs) const {
			return vec3(v + rhs.v);

		}

		vec3 operator-(const vec3 &rhs) const {
			return vec3(v - rhs.v);
			
		}
		Float max()const {
			return std::max(x(), std::max(y(), z()));
		}
		Float min()const {
			return std::min(x(), std::min(y(), z()));
		}
		static Float dotProduct(const vec3 &a, const vec3 &b) {
			return a.x() * b.x() + a.y()*b.y() + a.z()*b.z();
		}
		static vec3 crossProduct(const vec3 &v1, const vec3 &v2) {
			return vec3{ v1.y() * v2.z() - v1.z() * v2.y(),
					v1.z() * v2.x() - v1.x() * v2.z(),
					v1.x() * v2.y() - v1.y() * v2.x() };
		}
		Float lengthSquared()const {
			return vec3::dotProduct(*this, *this);
		}
		Float length()const {
			return sqrt(lengthSquared());
		}
		void normalize() {
			*this /= length();
		}
		const vec3 normalized()const {
			auto v = *this;
			v.normalize();
			return v;
		}
	};

	inline vec3 operator*(const vec3 &a, Float k) {
		vec3 _v(a);
		_v *= k;
		return _v;
	}

	inline vec3 operator*(Float k, const vec3 &a) {
		return a * k;
	}

	inline vec3 operator/(const vec3 &a, Float k) {
		vec3 _v(a);
		_v /= k;
		return _v;
	}


	inline Float clamp(Float x, Float a, Float b) {
		if (x < a)return a;
		if (x > b)return b;
		return x;
	}

	inline vec3 min(const vec3 &a, const vec3 & b) {
		return vec3(std::min(a.x(), b.x()),
			std::min(a.y(), b.y()),
			std::min(a.z(), b.z()));
	}

	inline vec3 max(const vec3 &a, const vec3 & b) {
		return vec3(std::max(a.x(), b.x()),
			std::max(a.y(), b.y()),
			std::max(a.z(), b.z()));
	}



	inline vec3 rgb(int r, int g, int b) {
		return vec3{ (Float)r, (Float)g, (Float)b } / 255.0;
	}

	/// axis is normalized
	inline vec3 rotate(const vec3 &x, const vec3 &axis, const Float angle) {
		Float s = sin(angle);
		Float c = cos(angle);
		Float oc = 1.0 - c;
		vec3 u({ oc * axis.x() * axis.x() + c, oc * axis.x() * axis.y() - axis.z() * s,
				oc * axis.z() * axis.x() + axis.y() * s });
		vec3 v({ oc * axis.x() * axis.y() + axis.z() * s, oc * axis.y() * axis.y() + c,
				oc * axis.y() * axis.z() - axis.x() * s });
		vec3 w({ oc * axis.z() * axis.x() - axis.y() * s, oc * axis.y() * axis.z() + axis.x() * s,
				oc * axis.z() * axis.z() + c });
		return vec3{ vec3::dotProduct(u, x), vec3::dotProduct(v, x), vec3::dotProduct(w, x) };
	}
	
}


#endif //VEC_H
