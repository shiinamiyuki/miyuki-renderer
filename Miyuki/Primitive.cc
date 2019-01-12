#include "Primitive.h"
#include "Scene.h"
using namespace Miyuki;
Primitive::Primitive()
{
}



Primitive::~Primitive()
{
}

void Miyuki::Sphere::intersect(const Ray &ray, Intersection &intersection)
{
	Float a = 1;
	Float b = 2 * vec3::dotProduct(ray.d, ray.o - center);//2 * (ray.direction * (ray.origin - center));
	Float c = (ray.o - center).lengthSquared() - radius * radius;
	Float delta = b * b - 4 * a * c;
	if (delta < 0) { return; }
	else {
		delta = sqrt(delta);
		auto d1 = (-b + delta) / (2 * a);
		auto d2 = (-b - delta) / (2 * a);
		Float d;
		if (d1 < 0)
			d = d2;
		else if (d2 < 0)
			d = d1;
		else {
			d = std::min(d1, d2);
		}
		if (d < eps) {
			return;
		}
		auto i = ray.o + ray.d * (d);
		auto norm = (i - center);
		norm.normalize();
		intersection.merge(this,d,norm);
	}
}

vec3 Miyuki::Sphere::getNormOfPoint(const vec3& p)
{
	return (p - center).normalized();
}

vec3 Miyuki::Sphere::randomPointOnObject(Seed*Xi)
{
	return radius * randomVectorInSphere(Xi) + center;
}

AABB Miyuki::Sphere::getBoundBox()
{
	vec3 v = vec3(radius, radius, radius);
	return AABB(center - v,center + v);
}

void Miyuki::Triangle::intersect(const Ray & ray, Intersection & intersection)
{
	vec3& vertex0 = vertices[0];
	vec3& vertex1 = vertices[1];
	vec3& vertex2 = vertices[2];
	vec3 edge1, edge2, h, s, q;
	Float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = vec3::crossProduct(ray.d, edge2);
	a = vec3::dotProduct(edge1, h);
	if (fabs(a)<eps) {
		return;
	}
	f = 1.0 / a;
	s = ray.o - vertex0;
	u = f * (vec3::dotProduct(s, h));
	if (u < 0.0 || u > 1.0)
		return;
	q = vec3::crossProduct(s, edge1);
	v = f * vec3::dotProduct(ray.d, q);
	if (v < 0.0 || u + v > 1.0)
		return;
	double t = f * vec3::dotProduct(edge2, q);
	if (t > eps) // ray intersection
	{
		auto n = norm;
		intersection.merge(this, t, n);
	}
	else
		return;
}

vec3 Miyuki::Triangle::randomPointOnObject(Seed*Xi)
{
	Float a = erand48(Xi);
	Float b = erand48(Xi) * (1 - a);
	return vertices[0] + a * (vertices[1] - vertices[0]) + b * (vertices[2] - vertices[0]);
}


Miyuki::SIMDTriangle::SIMDTriangle(std::vector<Triangle*> list)
{
	//assert(list.size() == simdVec::width());
	for (int i = 0; i < 8; i++) {
		trigs[i] = nullptr;
	}
	for (int i = 0; i < list.size(); i++) {
		for (int j = 0; j < 3; j++) {
			vertices[j].x[i] = list[i]->vertices[j].x();
			vertices[j].y[i] = list[i]->vertices[j].y();
			vertices[j].z[i] = list[i]->vertices[j].z();
		}
		norm.x[i] = list[i]->norm.x();
		norm.y[i] = list[i]->norm.y();
		norm.z[i] = list[i]->norm.z();
		trigs[i] = list[i];
	}
}


void Miyuki::SIMDTriangle::intersect(const SIMDRay & ray, Intersection & intersection)
{
	simdVec& vertex0 = vertices[0];
	simdVec& vertex1 = vertices[1];
	simdVec& vertex2 = vertices[2];
	simdVec edge1, edge2, h, s, q;
	simdVec::Scalar a, f, u, v,uv;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = simdVec::crossProduct(ray.d, edge2);
	a = simdVec::dotProduct(edge1, h);

	f = a;
	f.inv();
	s = ray.o - vertex0;
	u = f * (simdVec::dotProduct(s, h));

	q = simdVec::crossProduct(s, edge1);
	v = f * simdVec::dotProduct(ray.d, q);
	uv = u + v;
	simdVec::Scalar t = f * simdVec::dotProduct(edge2, q);
/*	static const simdVec::Scalar veps = simdVec::Scalar(0.01);
	static const simdVec::Scalar zero = simdVec::Scalar(0);
	static const simdVec::Scalar one = simdVec::Scalar(1);
	simdVec::Scalar r = one;
	a.abs();
	r = a > veps && u >= zero && u <= one && v >= zero && (u + v) <= one;
	for (int i = 0; i < simdVec::width(); i++) {
		if (r[i] && t[i] > eps) {
			auto n = vec3(norm.x[i], norm.y[i], norm.z[i]);
			intersection.merge(trigs[i], t[i], n);
		}
	}*/
	for (int i = 0; i < simdVec::width(); i++) {
		if ( fabs(a[i]) > 0.01 && u[i] >= 0.0 && u[i] <= 1.0 && v[i]>= 0.0 && uv[i] <= 1.0) {
			if (trigs[i] && t[i] > eps) {
				auto n = vec3(norm.x[i], norm.y[i], norm.z[i]);
				intersection.merge(trigs[i], t[i], n);
			}
		}
	}
}
