//
// Created by Shiina Miyuki on 2019/1/19.
//

#include <math/transform.h>

using namespace Miyuki;

Matrix4x4::Matrix4x4() {

}

Matrix4x4::Matrix4x4(Float _m[4][4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m[i][j] = _m[i][j];
		}
	}
}


Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& rhs) {
	for (int i = 0; i < 4; i++) {
		m[i] += rhs.m[i];
	}
	return *this;
}

Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& rhs) {
	for (int i = 0; i < 4; i++) {
		m[i] -= rhs.m[i];
	}
	return *this;
}

Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& rhs) {
	for (int i = 0; i < 4; i++) {
		m[i] *= rhs.m[i];
	}
	return *this;
}

Matrix4x4& Matrix4x4::operator/=(const Matrix4x4& rhs) {
	for (int i = 0; i < 4; i++) {
		m[i] /= rhs.m[i];
	}
	return *this;
}

Matrix4x4 Matrix4x4::identity() {
	static Float I[4][4] = { {1, 0, 0, 0},
							{0, 1, 0, 0},
							{0, 0, 1, 0},
							{0, 0, 0, 1} };
	return Matrix4x4(I);
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& rhs) const {
	auto m = *this;
	m += rhs;
	return m;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& rhs) const {
	auto m = *this;
	m -= rhs;
	return m;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& rhs) const {
	auto m = *this;
	m *= rhs;
	return m;
}

Matrix4x4 Matrix4x4::operator/(const Matrix4x4& rhs) const {
	auto m = *this;
	m /= rhs;
	return m;
}

Vec3f Matrix4x4::mult(const Vec3f& rhs) const {
	Vec3f v = Vec3f{ Vec3f::matDot(m[0], rhs),
					Vec3f::matDot(m[1], rhs),
					Vec3f::matDot(m[2], rhs) };
	v.w() = Vec3f::matDot(m[3], rhs);
	return v;
}

Matrix4x4 Matrix4x4::translation(const Vec3f& rhs) {
	Float m[4][4] = { {1, 0, 0, rhs.x()},
					 {0, 1, 0, rhs.y()},
					 {0, 0, 1, rhs.z()},
					 {0, 0, 0, 1} };
	return { m };
}

Matrix4x4 Matrix4x4::rotation(const Vec3f& axis, const Float angle) {
	const Float s = sin(angle);
	const Float c = cos(angle);
	const Float oc = Float(1.0) - c;
	Float m[4][4] = {
			{oc * axis.x() * axis.x() + c,
					oc * axis.x() * axis.y() - axis.z() * s,
					   oc * axis.z() * axis.x() + axis.y() * s, 0},
			{oc * axis.x() * axis.y() + axis.z() * s,
					oc * axis.y() * axis.y() + c,
					   oc * axis.y() * axis.z() - axis.x() * s, 0},

			{oc * axis.z() * axis.x() - axis.y() * s,
					oc * axis.y() * axis.z() + axis.x() * s,
					   oc * axis.z() * axis.z() + c,            0},
			{0,     0, 0,                                       1} };
	return Matrix4x4(m);
}

Matrix4x4 Matrix4x4::mult(const Matrix4x4& rhs) const {
	Matrix4x4 m;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m.m[i][j] = 0;
			for (int k = 0; k < 4; k++) {
				m.m[i][j] += (*this).m[i][k] * rhs.m[k][j];
			}
		}
	}
	return m;
}

Matrix4x4::Matrix4x4(const Vec3f& r1, const Vec3f& r2, const Vec3f& r3, const Vec3f& r4) {
	m[0] = r1;
	m[1] = r2;
	m[2] = r3;
	m[3] = r4;
}

bool Matrix4x4::inverse(const Matrix4x4& in, Matrix4x4& out) {
	double inv[16], det;
	int i;
	auto& m = in;
	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		out[i] = inv[i] * det;
	return true;
}


Transform::Transform()
	: translation(), rotation(), scale(1) {

}

Transform::Transform(const Vec3f & t, const Vec3f & r, Float s)
	: translation(t), rotation(r), scale(s) {

}

Vec3f Transform::apply(const Vec3f & _v, bool inverse) const {
	if (!inverse) {
		auto v = _v * scale;
		v = rotate(v, Vec3f(1, 0, 0), -rotation.y());
		v = rotate(v, Vec3f(0, 1, 0), rotation.x());
		v = rotate(v, Vec3f(0, 0, 1), rotation.z());
		return v + translation;
	}
	else {
		auto v = _v - translation;
		v = rotate(v, Vec3f(0, 0, 1), -rotation.z());
		v = rotate(v, Vec3f(0, 1, 0), -rotation.x());
		v = rotate(v, Vec3f(1, 0, 0), rotation.y());
		return v / scale;
	}
}

Vec3f Transform::apply_rotation(const Vec3f & _v, bool inverse) const {
	if (!inverse) {
		auto v = _v;
		v = rotate(v, Vec3f(1, 0, 0), -rotation.y());
		v = rotate(v, Vec3f(0, 1, 0), rotation.x());
		v = rotate(v, Vec3f(0, 0, 1), rotation.z());
		return v;
	}
	else {
		auto v = _v;
		v = rotate(v, Vec3f(0, 0, 1), -rotation.z());
		v = rotate(v, Vec3f(0, 1, 0), -rotation.x());
		v = rotate(v, Vec3f(1, 0, 0), rotation.y());
		return v;
	}
}

