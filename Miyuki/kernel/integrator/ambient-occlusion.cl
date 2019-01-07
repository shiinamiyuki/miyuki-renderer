#include "kernel/trace.h"
#include "kernel/common/bvh.h"
#include "kernel/common/util.h"
Vector AO(__global const RenderContext * ctx, Ray r, Seed * Xi) {
	Ray ray;
	size_t id = get_global_id(0);
	Intersection isct;
	initIntersection(&isct);
	if (ctx->samples <= 0) {
		
		ray.ro = r.ro;
		ray.rd = r.rd;
		
		findIntersection(&ray, ctx, &isct);
		if (!hit(&isct)) {
			ctx->opt[id].pos = vec3(0, 0, 0);
			ctx->opt[id].refl = vec3(0, 0, 0);
			ctx->opt[id].norm = vec3(0, 0, 0);
			ctx->opt[id].id = -1;
			return vec3(0, 0, 0);
		}
		__global const Primitive * object = ctx->objects + isct.id;
		ray.ro += isct.distance * ray.rd;
	//	ctx->opt[id].refl = object->material.diffuse;
		ctx->opt[id].norm = isct.norm;
		ctx->opt[id].pos = ray.ro;
		ctx->opt[id].id = isct.id;
	}
	ray.ro = ctx->opt[id].pos;
	Vector norm = ctx->opt[id].norm;
	ray.rd = cosineWeightedSampling(&norm, Xi);
	isct.exclude = ctx->opt[id].id;
	findIntersection(&ray, ctx, &isct);
	Vector color;
	if (!hit(&isct))
		color = vec3(1, 1, 1);
	else {
		color = vec3(0, 0, 0);// vec3(1, 1, 1)* min(1.0, isct.distance / 400.0);
	}
	return color;
}
Vector integrator(__global const RenderContext * ctx, Ray ray, Seed *Xi) {
	return AO(ctx, ray, Xi);
}