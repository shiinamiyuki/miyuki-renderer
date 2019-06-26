//
// Created by Shiina Miyuki on 2019/1/19.
//

#ifndef MIYUKI_TRANSFORM_H
#define MIYUKI_TRANSFORM_H

#include "miyuki.h"
#include "vec.hpp"
#include "func.h"

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
        Vec3f apply(const Vec3f &, bool inverse = false) const;

        Vec3f apply_rotation(const Vec3f &, bool inverse = false) const;
    };
	inline void to_json(json& j, const Transform& T) {
		j = json{
			{"rotation", T.rotation},
			{"translation", RadiansToDegrees(T.translation)},
			{"scale", T.scale} 
		};
	}

	inline void from_json(const json& j, Transform& T) {
		T = Transform();
		if(j.contains("rotation"))
			j.at("rotation").get_to(T.rotation);
		T.rotation = DegreesToRadians(T.rotation);
		if (j.contains("translation"))
			j.at("translation").get_to(T.translation);
		if (j.contains("scale"))
			j.at("scale").get_to(T.scale);
	}

    class CoordinateSystem {
        Vec3f localX, localY, normal;
    public:
        CoordinateSystem() {}

        CoordinateSystem(const Vec3f &N) : normal(N) {
            const auto &w = normal;
            localX = Vec3f::cross((abs(w.x()) > 0.1) ? Vec3f{0, 1, 0} : Vec3f{1, 0, 0}, w);
            localX.normalize();
            localY = Vec3f::cross(w, localX);
            localY.normalize();
        }

        Vec3f worldToLocal(const Vec3f &v) const {
            return Vec3f(Vec3f::dot(localX, v), Vec3f::dot(localY, v), Vec3f::dot(normal, v)).normalized();
        }

        Vec3f localToWorld(const Vec3f &v) const {
            return (v.x() * localX + v.y() * localY + v.z() * normal).normalized();
        }

		bool valid()const {
			return normal != Vec3f();
		}
		Vec3f N()const {
			return normal;
		}
    };
}
#endif //MIYUKI_TRANSFORM_H
