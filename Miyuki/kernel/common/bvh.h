#ifndef BVH_H
#define BVH_H
#include "kernel/trace.h"
#include "kernel/common/intersect.h"
#define swap(a,b) do{float t = a;a =b;b = t;}while(0)
bool RayAABBIntersect(const Ray *ray, __global const AABB *box) {
	Vector t1 = (box->min - ray->ro) * ray->invd;
	Vector t2 = (box->max - ray->ro) * ray->invd;
	float tmin = t1.x;
	float tmax = t2.x;
	if (tmin > tmax)
		swap(tmin, tmax);
	float tymin = t1.y;
	float tymax = t2.y;
	if (tymin > tymax)
		swap(tymin, tymax);
	if (tmin > tymax || tymin > tmax)
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;
	float tzmin = t1.z;
	float tzmax = t2.z;

	if (tzmin > tzmax) swap(tzmin, tzmax);
	if (tmin > tzmax || tzmin > tmax)
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;
	return tmin > eps || tmax > eps;
}


void bvhTraversal(const Ray* _ray, __global const RenderContext * ctx, Intersection * isct) {
	int stack[40];
	int sp = 0;
	__global const BVHNode* tree = ctx->nodes;
	__global const Primitive * objects = ctx->objects;
	__global int * prim = ctx->primitivePool;
	const int root = 0;
	Ray ray;
	ray.ro = _ray->ro;
	ray.rd = _ray->rd;
	ray.invd = vec3(1, 1, 1) / ray.rd;
	if (!RayAABBIntersect(&ray, &tree[root].box)) {
		return;
	}
	stack[sp++] = root;
	while (sp > 0) {
		int ptr = stack[--sp];
		__global const BVHNode*node = &tree[ptr];
		if (ptr < 0)continue;
		if (node->begin == node->end) {
			int L = node->left;
			int R = node->right;
			if (L > 0 && RayAABBIntersect(&ray, &tree[L].box))
				stack[sp++] = L;
			if (R > 0 && RayAABBIntersect(&ray, &tree[R].box))
				stack[sp++] = R;
		}
		else {
			int end = node->end;
			int i = node->begin;
			for (; i < end; i++) {
				intersectPrimitive(&ray, objects + prim[i], isct);
			}
		}
	}
}
#endif