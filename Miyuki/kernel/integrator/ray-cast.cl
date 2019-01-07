#include "kernel/trace.h"
#include "kernel/common/bvh.h"
#include "kernel/common/util.h"

Vector raycast(__global const RenderContext * ctx, Ray r, Seed * Xi) {
	Ray ray;
	ray.ro = r.ro;
	ray.rd = r.rd;
	Intersection isct;
	initIntersection(&isct);
	findIntersection(&ray, ctx, &isct);

	if (!hit(&isct)) {
		return vec3(0, 0, 0);
	}
	__global const Primitive * object = ctx->objects + isct.id;
	return (ctx->materials[object->material].ambient
		+ ctx->materials[object->material].diffuse* max(0.2, dot(isct.norm, normalize(vec3(0.4, 1, 0.2))))
		)* (dot(isct.norm, ray.rd) < 0.0f);
}
Vector integrator(__global const RenderContext * ctx, Ray r, Seed * Xi) {
	return raycast(ctx, r, Xi);
}