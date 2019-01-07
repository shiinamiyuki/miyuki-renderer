#ifndef RAND_H
#define RAND_H
#include "kernel/trace.h"
#include "kernel/bxdf.h"
inline float rand(Seed *seed) {
	seed[0] = 36969 * ((seed[0]) & 65535) + ((seed[0]) >> 16);
	seed[1] = 18000 * ((seed[1]) & 65535) + ((seed[1]) >> 16);

	unsigned int ires = ((seed[0]) << 16) + (seed[1]);

	union {
		float f;
		unsigned int ui;
	} res;
	res.ui = (ires & 0x007fffff) | 0x40000000;

	return (res.f - 2.f) / 2.f;
}

inline Vector cosineWeightedSampling(const Vector *norm, Seed *Xi) {//cosine weighted
	float r1 = 2 * (float)pi * rand(Xi), r2 = rand(Xi), r2s = (float)sqrt(r2);
	float x = norm->x;
	Vector u = normalize(cross((fabs(x) > 0.1)
		? vec3(0, 1, 0)
		: vec3(1, 0, 0), *norm));
	Vector v = cross(*norm, u);
	Vector r = normalize(u * cos(r1) * r2s + v * sin(r1) * r2s + *norm * sqrt(1 - r2));
	return r;
}
// http://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
inline Vector GGXImportanceSampling(float roughness, const Vector *norm, Seed *Xi) {
	float a = rand(Xi);
	float b = rand(Xi);
	float tm = atan(roughness * sqrt(a / (1 - a)));
	float phim = 2 * pi * b;
	float x = norm->x;
	Vector u = normalize(cross((fabs(x) > 0.1)
		? vec3(0, 1, 0)
		: vec3(1, 0, 0), *norm));
	Vector v = cross(*norm, u);
	Vector r = normalize(u * cos(phim) * sin(tm) + v * sin(phim)*sin(tm) + *norm * cos(tm));
	return r;
}

Vector randomVectorInSphereU1U2(float u1, float u2) {
	const float zz = 1.f - 2.f * u1;
	const float r = sqrt(max(0.f, 1.f - zz * zz));
	const float phi = 2.f * pi * u2;
	const float xx = r * cos(phi);
	const float yy = r * sin(phi);
	return vec3(xx, yy, zz);
}
Vector randomVectorInSphere(Seed *Xi) {
	const float u1 = rand(Xi);
	const float u2 = rand(Xi);
	const float zz = 1.f - 2.f * u1;
	const float r = sqrt(max(0.f, 1.f - zz * zz));
	const float phi = 2.f * pi * u2;
	const float xx = r * cos(phi);
	const float yy = r * sin(phi);
	return vec3(xx, yy, zz);
}
inline Vector reflect(const Vector n, Vector v) {
	return  (v - (2.0f * (dot(v, n))* n ));
}
Vector avgBySamples(Vector avg, Vector x, float s) {
	return (avg * s + x) / (1 + s);
}
Vector integrator(__global const RenderContext * ctx, Ray ray, Seed *Xi);
__kernel
void render (
	__global RenderContext * ctx,
	__global Material * materials,
	__global Primitive* objects,
	__global Seed * Xi,
	__global BVHNode* nodes,	
	__global int * primPool,
	__global int * lights,
	__global Pixel * image,
	__global OptionalBuffer * opt) {
	int w = ctx->width;
	int h = ctx->height;
	int idx = get_global_id(0);
	if (idx >= ctx->size) {
		return;
	}
	ctx->opt = opt;
	ctx->objects = objects;
	ctx->nodes = nodes;
	ctx->primitivePool = primPool;
	ctx->lights = lights;
	ctx->materials = materials;
	if (ctx->samples < 0)
		ctx->samples = 0;
	int i, j;
	float x, y;
	float dx, dy;
	i = idx % w;
	j = idx / w;

	dx = 2.0f / (float)w;
	dy = 2.0f / (float)h;
	x = (float)(idx % w) / w;
	y = ((float)(idx) / w) / h;
	y = 1 - y;
	x = 2 * x - 1;
	y = 2 * y - 1;


	Seed seed[2];
	seed[0] = Xi[idx * 2];
	seed[1] = Xi[idx * 2 + 1];
	float du = (2 * rand(seed) - 1);
	float dv = (2 * rand(seed) - 1);
	x += du * dx / 2.0f;
	y += dv * dy / 2.0f;
//	du = -fabs(du) + 1;
//  dv = -fabs(dv) + 1;
	Ray ray;
	ray.ro = ctx->origin;
	float z = 2.0 / tan(pi / 2 / 2);
	Vector dir = normalize(vec3(x, y, 0) - vec3(0, 0, -z));
	dir = vecRotate(dir, vec3(1, 0, 0), ctx->dir.y);
	dir = vecRotate(dir, vec3(0, 1, 0), ctx->dir.x);
	ray.rd = dir;
    Vector color = integrator(ctx, ray, seed);
	if (isnan(color.x))
		color.x = 0;
	if (isnan(color.y))
		color.y = 0;
	if (isnan(color.z))
		color.z = 0;
    image[idx] = avgBySamples(image[idx], color, ctx->samples);
	Xi[idx * 2] = seed[0];
	Xi[idx * 2 + 1] = seed[1];

}
#endif