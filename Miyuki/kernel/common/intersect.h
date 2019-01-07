#ifndef ISCT_H
#define ISCT_H
#include "kernel/trace.h"
typedef struct Intersection {
	int id;
	int exclude;
	float distance;
	Vector norm;
} Intersection;
typedef struct Ray {
	Vector ro, rd;
	Vector invd;
} Ray;


inline void initIntersection(Intersection *intersection) {
	intersection->distance = inf;
	intersection->id = -1;
	intersection->exclude = -1;
}
inline void resetIntersection(Intersection *intersection) {
	if(intersection->id >= 0)
		intersection->exclude = intersection->id;
	intersection->id = -1;
	intersection->distance = inf;
}

inline void updateIntersection(Intersection *intersection,
	int id,
	float distance,
	Vector *norm) {
	if (distance < intersection->distance && id != intersection->exclude) {
		intersection->distance = distance;
		intersection->id = id;
		intersection->norm = *norm;
	}
}
inline void intersectSphere(
	__global const Primitive *object,
	const Ray *ray,
	Intersection *intersection) {
	const float r = object->sphere.radius;
	Vector dir = ray->ro - object->sphere.center;
	float b = 2 * dot(ray->rd, dir);//2 * (ray.direction * (ray.origin - center));
	float c = dot(dir, dir)
		- r * r;
	float delta = b * b - 4 * c;
	if (delta < 0) { return; }
	else {
		delta = sqrt(delta);
		float d1 = (delta - b) / (2);//d1 > d2
		float d2 = (-b - delta) / (2);
		float d;
		if (d2 < 0)
			d = d1;
		else
			d = d2;
		if (d < eps) {
			return;
		}
		Vector i = ray->ro + d * ray->rd;
		Vector norm = (i - object->sphere.center) / r;
		updateIntersection(intersection, object->id, d, &norm);
	}
}
inline bool hit(Intersection *intersection) {
	return intersection->id >= 0;
}
//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
inline void intersectTriangle(
	__global const Primitive *object,
	const Ray *ray,
	Intersection *intersection) {
	Vector vertex0 = object->triangle.vertex0;
	Vector vertex1 = object->triangle.vertex1;
	Vector vertex2 = object->triangle.vertex2;
	Vector edge1, edge2, h, s, q;
	edge1 = object->triangle.edge1;
	edge2 = object->triangle.edge2;
	float a, f, u, v;

	h = cross(ray->rd, edge2);
	a = dot(edge1, h);
	
	f = 1.0f / a;
	s = ray->ro - vertex0;
	u = f * (dot(s, h));
	
	q = cross(s, edge1);
	v = f * dot(ray->rd, q);
	float t = f * dot(edge2, q);
	if (fabs(a) < eps) {
		return;
	}
	if (v < 0.0 || u + v > 1)
		return;
	if (u < 0.0 || u > 1.0)
		return;
	if (t > eps) {
		Vector norm = object->triangle.norm;
		updateIntersection(intersection, object->id, t, &norm);
	}
}

void intersectPrimitive(const Ray* ray, __global const Primitive * object, Intersection*isct) {
	if (object->type == TYPE_SPHERE) {
		intersectSphere(object, ray, isct);
	}
	else {
		intersectTriangle(object, ray, isct);
	}
}
void findIntersectionBruteForce(const Ray* ray, __global const  RenderContext * ctx, Intersection * isct) {
	resetIntersection(isct);
	int cnt = ctx->objectCount;
	__global const Primitive * object = ctx->objects;
	for (int i = cnt; i; i--) {
		intersectPrimitive(ray, object, isct);
		object++;
	}
}
void bvhTraversal(const Ray* ray, __global const RenderContext * ctx, Intersection * isct);
void findIntersection(const Ray* ray, __global const RenderContext * ctx, Intersection * isct) {
	resetIntersection(isct);
	bvhTraversal(ray, ctx, isct);
}

#endif